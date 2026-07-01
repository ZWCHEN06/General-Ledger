#include "DatabaseManager.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace {
QString prepareDatabasePath(const QString &databaseFileName, bool *ok)
{
    if (ok) {
        *ok = false;
    }

    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataPath.isEmpty()) {
        qWarning().noquote() << "Failed to resolve app data location for SQLite database.";
        return QString();
    }

    QDir appDataDir(appDataPath);
    if (!appDataDir.exists() && !QDir().mkpath(appDataPath)) {
        qWarning().noquote() << "Failed to create SQLite database directory:" << appDataPath;
        return QString();
    }

    const QString databasePath = appDataDir.filePath(databaseFileName);
    const QString legacyDatabasePath = QDir::current().absoluteFilePath(databaseFileName);

    if (!QFileInfo::exists(databasePath)
        && QFileInfo::exists(legacyDatabasePath)
        && QDir::cleanPath(databasePath) != QDir::cleanPath(legacyDatabasePath)) {
        if (!QFile::copy(legacyDatabasePath, databasePath)) {
            qWarning().noquote() << "Failed to migrate legacy SQLite database from"
                                 << legacyDatabasePath << "to" << databasePath;
            return QString();
        }

        qInfo().noquote() << "Migrated legacy SQLite database from"
                          << legacyDatabasePath << "to" << databasePath;
    }

    if (ok) {
        *ok = true;
    }
    return databasePath;
}
}

DatabaseManager::DatabaseManager()
{
    bool databasePathReady = false;
    const QString databasePath = prepareDatabasePath(QString::fromLatin1(DatabaseFileName), &databasePathReady);

    if (QSqlDatabase::contains(ConnectionName)) {
        m_database = QSqlDatabase::database(ConnectionName);
    } else {
        m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), ConnectionName);
    }

    if (databasePathReady && !databasePath.isEmpty() && !m_database.isOpen()) {
        m_database.setDatabaseName(databasePath);
    }

    qInfo().noquote() << "SQLite database path:" << m_database.databaseName();
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase()
{
    if (m_database.isOpen()) {
        return true;
    }

    if (m_database.databaseName().trimmed().isEmpty()) {
        qWarning().noquote() << "SQLite database path is empty.";
        return false;
    }

    return m_database.open();
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::isOpen() const
{
    return m_database.isOpen();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_database;
}

QString DatabaseManager::lastErrorMessage() const
{
    return m_lastErrorMessage;
}

bool DatabaseManager::initializeTables()
{
    m_lastErrorMessage.clear();

    if (!openDatabase()) {
        m_lastErrorMessage = QStringLiteral("Failed to open SQLite database: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "打开数据库失败:" << m_database.lastError().text();
        return false;
    }

    if (!ensureTransactionsTable()) {
        return false;
    }

    return migrateDatabase();
}

bool DatabaseManager::ensureTransactionsTable()
{
    QSqlQuery query(m_database);
    const bool success = query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type TEXT NOT NULL,
            amount REAL NOT NULL,
            category TEXT NOT NULL,
            date TEXT NOT NULL,
            note TEXT,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        )
    )"));

    if (!success) {
        m_lastErrorMessage = QStringLiteral("Failed to initialize transactions table: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "初始化 transactions 表失败:" << query.lastError().text();
        return false;
    }

    return true;
}

int DatabaseManager::currentUserVersion()
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA user_version"))) {
        m_lastErrorMessage = QStringLiteral("Failed to read SQLite user_version: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to read SQLite user_version:" << query.lastError().text();
        return -1;
    }

    if (!query.next()) {
        m_lastErrorMessage = QStringLiteral("Failed to read SQLite user_version: empty result");
        qWarning().noquote() << "Failed to read SQLite user_version: empty result";
        return -1;
    }

    return query.value(0).toInt();
}

bool DatabaseManager::setUserVersion(int version)
{
    QSqlQuery query(m_database);
    const bool success = query.exec(QStringLiteral("PRAGMA user_version = %1").arg(version));
    if (!success) {
        m_lastErrorMessage = QStringLiteral("Failed to set SQLite user_version to %1: %2")
                                 .arg(version)
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to set SQLite user_version:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::migrateDatabase()
{
    const int version = currentUserVersion();
    if (version < 0) {
        return false;
    }

    if (version > CurrentSchemaVersion) {
        m_lastErrorMessage = QStringLiteral("Unsupported SQLite schema version %1; current supported version is %2")
                                 .arg(version)
                                 .arg(CurrentSchemaVersion);
        qWarning().noquote() << "Unsupported SQLite schema version:" << version
                             << "current supported version:" << CurrentSchemaVersion;
        return false;
    }

    if (version < 1 && !migrateToVersion1()) {
        return false;
    }

    return true;
}

bool DatabaseManager::migrateToVersion1()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 1: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "Failed to start SQLite schema migration transaction:"
                             << m_database.lastError().text();
        return false;
    }

    const auto rollbackMigration = [this]() {
        if (!m_database.rollback()) {
            qWarning().noquote() << "Failed to roll back SQLite schema migration:"
                                 << m_database.lastError().text();
        }
    };

    if (!createCategoriesTable() || !setUserVersion(1)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 1: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "Failed to commit SQLite schema migration:"
                             << m_database.lastError().text();
        rollbackMigration();
        return false;
    }

    return true;
}

bool DatabaseManager::createCategoriesTable()
{
    QSqlQuery query(m_database);
    const bool tableCreated = query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            type TEXT NOT NULL CHECK (type IN ('income', 'expense')),
            is_default INTEGER NOT NULL DEFAULT 0,
            sort_order INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            UNIQUE(name, type)
        )
    )"));

    if (!tableCreated) {
        m_lastErrorMessage = QStringLiteral("Failed to create categories table during schema migration to version 1: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to create categories table:" << query.lastError().text();
        return false;
    }

    const bool indexCreated = query.exec(QStringLiteral(R"(
        CREATE INDEX IF NOT EXISTS idx_categories_type_sort
        ON categories(type, sort_order, id)
    )"));

    if (!indexCreated) {
        m_lastErrorMessage = QStringLiteral("Failed to create categories index during schema migration to version 1: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to create categories index:" << query.lastError().text();
        return false;
    }

    return true;
}
