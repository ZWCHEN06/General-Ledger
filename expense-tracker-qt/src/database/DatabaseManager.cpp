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

bool DatabaseManager::initializeTables()
{
    if (!openDatabase()) {
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
        qWarning().noquote() << "初始化 transactions 表失败:" << query.lastError().text();
        return false;
    }

    return true;
}

int DatabaseManager::currentUserVersion() const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA user_version"))) {
        qWarning().noquote() << "Failed to read SQLite user_version:" << query.lastError().text();
        return -1;
    }

    if (!query.next()) {
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

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!createCategoriesTable()
        || !addTransactionCategoryIdColumn()
        || !seedDefaultCategories(now)
        || !backfillCategoriesFromTransactions(now)
        || !backfillTransactionCategoryIds()
        || !setUserVersion(1)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
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
            type TEXT NOT NULL CHECK (type IN ('income', 'expense')),
            name TEXT NOT NULL,
            sort_order INTEGER NOT NULL DEFAULT 0,
            is_default INTEGER NOT NULL DEFAULT 0 CHECK (is_default IN (0, 1)),
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            UNIQUE(type, name)
        )
    )"));

    if (!tableCreated) {
        qWarning().noquote() << "Failed to create categories table:" << query.lastError().text();
        return false;
    }

    const bool indexCreated = query.exec(QStringLiteral(R"(
        CREATE INDEX IF NOT EXISTS idx_categories_type_sort
        ON categories(type, sort_order, id)
    )"));

    if (!indexCreated) {
        qWarning().noquote() << "Failed to create categories index:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::addTransactionCategoryIdColumn()
{
    if (columnExists(QStringLiteral("transactions"), QStringLiteral("category_id"))) {
        return true;
    }

    QSqlQuery query(m_database);
    const bool success = query.exec(QStringLiteral(R"(
        ALTER TABLE transactions ADD COLUMN category_id INTEGER
    )"));

    if (!success) {
        qWarning().noquote() << "Failed to add transactions.category_id:"
                             << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::seedDefaultCategories(const QString &timestamp)
{
    struct DefaultCategorySeed
    {
        QString type;
        QString name;
        int sortOrder;
    };

    const DefaultCategorySeed defaultCategories[] = {
        {QStringLiteral("expense"), QStringLiteral("餐饮"), 0},
        {QStringLiteral("expense"), QStringLiteral("交通"), 10},
        {QStringLiteral("expense"), QStringLiteral("购物"), 20},
        {QStringLiteral("expense"), QStringLiteral("娱乐"), 30},
        {QStringLiteral("income"), QStringLiteral("工资"), 0},
        {QStringLiteral("income"), QStringLiteral("奖金"), 10},
        {QStringLiteral("income"), QStringLiteral("兼职"), 20},
        {QStringLiteral("income"), QStringLiteral("其他"), 30}
    };

    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        INSERT OR IGNORE INTO categories (
            type,
            name,
            sort_order,
            is_default,
            created_at,
            updated_at
        ) VALUES (
            :type,
            :name,
            :sort_order,
            1,
            :created_at,
            :updated_at
        )
    )"))) {
        qWarning().noquote() << "Failed to prepare default category seed SQL:"
                             << query.lastError().text();
        return false;
    }

    for (const DefaultCategorySeed &category : defaultCategories) {
        query.bindValue(QStringLiteral(":type"), category.type);
        query.bindValue(QStringLiteral(":name"), category.name);
        query.bindValue(QStringLiteral(":sort_order"), category.sortOrder);
        query.bindValue(QStringLiteral(":created_at"), timestamp);
        query.bindValue(QStringLiteral(":updated_at"), timestamp);

        if (!query.exec()) {
            qWarning().noquote() << "Failed to seed default category:"
                                 << category.type << category.name << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DatabaseManager::backfillCategoriesFromTransactions(const QString &timestamp)
{
    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        INSERT OR IGNORE INTO categories (
            type,
            name,
            sort_order,
            is_default,
            created_at,
            updated_at
        )
        SELECT
            type,
            TRIM(category),
            1000,
            0,
            :created_at,
            :updated_at
        FROM transactions
        WHERE type IN ('income', 'expense')
          AND TRIM(category) <> ''
        GROUP BY type, TRIM(category)
    )"))) {
        qWarning().noquote() << "Failed to prepare historical category backfill SQL:"
                             << query.lastError().text();
        return false;
    }

    query.bindValue(QStringLiteral(":created_at"), timestamp);
    query.bindValue(QStringLiteral(":updated_at"), timestamp);

    if (!query.exec()) {
        qWarning().noquote() << "Failed to backfill categories from transactions:"
                             << query.lastError().text();
        return false;
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
            WHERE categories.type = transactions.type
              AND categories.name = TRIM(transactions.category)
        )
        WHERE category_id IS NULL
          AND type IN ('income', 'expense')
          AND TRIM(category) <> ''
    )"));

    if (!success) {
        qWarning().noquote() << "Failed to backfill transactions.category_id:"
                             << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::columnExists(const QString &tableName, const QString &columnName) const
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA table_info(%1)").arg(tableName))) {
        qWarning().noquote() << "Failed to inspect SQLite table columns:"
                             << tableName << query.lastError().text();
        return false;
    }

    while (query.next()) {
        if (query.value(1).toString() == columnName) {
            return true;
        }
    }

    return false;
}
