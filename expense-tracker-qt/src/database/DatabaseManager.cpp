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

bool DatabaseManager::initializeTables()
{
    if (!openDatabase()) {
        qWarning().noquote() << "打开数据库失败:" << m_database.lastError().text();
        return false;
    }

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
        qWarning().noquote() << "初始化 transactions 表失败:" << query.lastError().text();
        return false;
    }

    return true;
}
