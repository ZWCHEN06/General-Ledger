#include "DatabaseManager.h"

#include <QDebug>
#include <QDateTime>
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

    if (version < 2 && !migrateToVersion2()) {
        return false;
    }

    if (version < 3 && !migrateToVersion3()) {
        return false;
    }

    if (version < 4 && !migrateToVersion4()) {
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

bool DatabaseManager::migrateToVersion2()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 2: %1")
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

    if (!addTransactionCategoryIdColumn() || !setUserVersion(2)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 2: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "Failed to commit SQLite schema migration:"
                             << m_database.lastError().text();
        rollbackMigration();
        return false;
    }

    return true;
}

bool DatabaseManager::migrateToVersion3()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 3: %1")
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

    if (!seedDefaultCategories() || !setUserVersion(3)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 3: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "Failed to commit SQLite schema migration:"
                             << m_database.lastError().text();
        rollbackMigration();
        return false;
    }

    return true;
}

bool DatabaseManager::migrateToVersion4()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 4: %1")
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

    if (!backfillTransactionCategoryIds() || !setUserVersion(4)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 4: %1")
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

bool DatabaseManager::seedDefaultCategories()
{
    struct DefaultCategorySeed
    {
        QString name;
        QString type;
        int sortOrder;
    };

    const DefaultCategorySeed defaultCategories[] = {
        {QStringLiteral("餐饮"), QStringLiteral("expense"), 0},
        {QStringLiteral("交通"), QStringLiteral("expense"), 1},
        {QStringLiteral("购物"), QStringLiteral("expense"), 2},
        {QStringLiteral("学习"), QStringLiteral("expense"), 3},
        {QStringLiteral("娱乐"), QStringLiteral("expense"), 4},
        {QStringLiteral("住房"), QStringLiteral("expense"), 5},
        {QStringLiteral("医疗"), QStringLiteral("expense"), 6},
        {QStringLiteral("其他"), QStringLiteral("expense"), 7},
        {QStringLiteral("生活费"), QStringLiteral("income"), 0},
        {QStringLiteral("工资"), QStringLiteral("income"), 1},
        {QStringLiteral("奖学金"), QStringLiteral("income"), 2},
        {QStringLiteral("兼职"), QStringLiteral("income"), 3},
        {QStringLiteral("投资"), QStringLiteral("income"), 4},
        {QStringLiteral("其他"), QStringLiteral("income"), 5}
    };

    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        INSERT OR IGNORE INTO categories (
            name,
            type,
            is_default,
            sort_order,
            created_at,
            updated_at
        ) VALUES (
            :name,
            :type,
            1,
            :sort_order,
            :created_at,
            :updated_at
        )
    )"))) {
        m_lastErrorMessage = QStringLiteral("Failed to prepare default category seed SQL during schema migration to version 3: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to prepare default category seed SQL:"
                             << query.lastError().text();
        return false;
    }

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    for (const DefaultCategorySeed &category : defaultCategories) {
        query.bindValue(QStringLiteral(":name"), category.name);
        query.bindValue(QStringLiteral(":type"), category.type);
        query.bindValue(QStringLiteral(":sort_order"), category.sortOrder);
        query.bindValue(QStringLiteral(":created_at"), now);
        query.bindValue(QStringLiteral(":updated_at"), now);

        if (!query.exec()) {
            m_lastErrorMessage = QStringLiteral("Failed to seed default category '%1' during schema migration to version 3: %2")
                                     .arg(category.name)
                                     .arg(query.lastError().text());
            qWarning().noquote() << "Failed to seed default category:"
                                 << category.type << category.name << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::backfillTransactionCategoryIds()
{
    QSqlQuery query(m_database);
    const bool success = query.exec(QStringLiteral(R"(
        UPDATE transactions
        SET category_id = (
            SELECT categories.id
            FROM categories
            WHERE categories.name = transactions.category
              AND categories.type = transactions.type
        )
        WHERE category_id IS NULL
          AND EXISTS (
              SELECT 1
              FROM categories
              WHERE categories.name = transactions.category
                AND categories.type = transactions.type
          )
    )"));

    if (!success) {
        m_lastErrorMessage = QStringLiteral("Failed to backfill transactions.category_id during schema migration to version 4: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to backfill transactions.category_id:"
                             << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::addTransactionCategoryIdColumn()
{
    bool exists = false;
    if (!columnExists(QStringLiteral("transactions"), QStringLiteral("category_id"), &exists)) {
        return false;
    }

    if (exists) {
        return true;
    }

    QSqlQuery query(m_database);
    const bool success = query.exec(QStringLiteral(R"(
        ALTER TABLE transactions ADD COLUMN category_id INTEGER
    )"));

    if (!success) {
        m_lastErrorMessage = QStringLiteral("Failed to add transactions.category_id during schema migration to version 2: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to add transactions.category_id:"
                             << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::columnExists(const QString &tableName, const QString &columnName, bool *exists)
{
    if (exists) {
        *exists = false;
    }

    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA table_info(%1)").arg(tableName))) {
        m_lastErrorMessage = QStringLiteral("Failed to inspect SQLite table columns for %1: %2")
                                 .arg(tableName)
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to inspect SQLite table columns:"
                             << tableName << query.lastError().text();
        return false;
    }

    while (query.next()) {
        if (query.value(1).toString() == columnName) {
            if (exists) {
                *exists = true;
            }
            return true;
        }
    }

    return true;
}
