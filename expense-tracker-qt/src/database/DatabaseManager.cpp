#include "DatabaseManager.h"

#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>

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

    if (!enableForeignKeys()) {
        return false;
    }

    if (!ensureTransactionsTable()) {
        return false;
    }

    return migrateDatabase();
}

bool DatabaseManager::enableForeignKeys()
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA foreign_keys = ON"))) {
        m_lastErrorMessage = QStringLiteral("Failed to enable SQLite foreign keys: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to enable SQLite foreign keys:" << query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral("PRAGMA foreign_keys"))) {
        m_lastErrorMessage = QStringLiteral("Failed to verify SQLite foreign keys: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to verify SQLite foreign keys:" << query.lastError().text();
        return false;
    }

    if (!query.next() || query.value(0).toInt() != 1) {
        m_lastErrorMessage = QStringLiteral("Failed to enable SQLite foreign keys: PRAGMA foreign_keys is not ON");
        qWarning().noquote() << "Failed to enable SQLite foreign keys: PRAGMA foreign_keys is not ON";
        return false;
    }

    return true;
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

    if (version < 5 && !migrateToVersion5()) {
        return false;
    }

    if (version < 6 && !migrateToVersion6()) {
        return false;
    }

    if (version < 7 && !migrateToVersion7()) {
        return false;
    }

    if (version < 8 && !migrateToVersion8()) {
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

bool DatabaseManager::migrateToVersion5()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 5: %1")
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

    if (!createWeeklyBudgetsTable() || !setUserVersion(5)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 5: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "Failed to commit SQLite schema migration:"
                             << m_database.lastError().text();
        rollbackMigration();
        return false;
    }

    return true;
}

bool DatabaseManager::migrateToVersion6()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 6: %1")
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

    if (!createSubcategoriesTable() || !setUserVersion(6)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 6: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "Failed to commit SQLite schema migration:"
                             << m_database.lastError().text();
        rollbackMigration();
        return false;
    }

    return true;
}

bool DatabaseManager::migrateToVersion7()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 7: %1")
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

    if (!addTransactionSubcategoryColumns() || !setUserVersion(7)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 7: %1")
                                 .arg(m_database.lastError().text());
        qWarning().noquote() << "Failed to commit SQLite schema migration:"
                             << m_database.lastError().text();
        rollbackMigration();
        return false;
    }

    return true;
}

bool DatabaseManager::migrateToVersion8()
{
    if (!m_database.transaction()) {
        m_lastErrorMessage = QStringLiteral("Failed to start SQLite schema migration to version 8: %1")
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

    if (!seedDefaultSubcategories() || !setUserVersion(8)) {
        rollbackMigration();
        return false;
    }

    if (!m_database.commit()) {
        m_lastErrorMessage = QStringLiteral("Failed to commit SQLite schema migration to version 8: %1")
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

bool DatabaseManager::createSubcategoriesTable()
{
    QSqlQuery query(m_database);
    const bool tableCreated = query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS subcategories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            category_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            is_default INTEGER NOT NULL DEFAULT 0,
            sort_order INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            UNIQUE(category_id, name),
            FOREIGN KEY(category_id) REFERENCES categories(id)
        )
    )"));

    if (!tableCreated) {
        m_lastErrorMessage = QStringLiteral("Failed to create subcategories table during schema migration to version 6: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to create subcategories table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::createWeeklyBudgetsTable()
{
    QSqlQuery query(m_database);
    const bool tableCreated = query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS weekly_budgets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            week_start_date TEXT NOT NULL,
            category_id INTEGER NOT NULL,
            amount REAL NOT NULL CHECK (amount >= 0),
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            UNIQUE(week_start_date, category_id),
            FOREIGN KEY(category_id) REFERENCES categories(id)
        )
    )"));

    if (!tableCreated) {
        m_lastErrorMessage = QStringLiteral("Failed to create weekly_budgets table during schema migration to version 5: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to create weekly_budgets table:" << query.lastError().text();
        return false;
    }

    const bool weekIndexCreated = query.exec(QStringLiteral(R"(
        CREATE INDEX IF NOT EXISTS idx_weekly_budgets_week
        ON weekly_budgets(week_start_date)
    )"));

    if (!weekIndexCreated) {
        m_lastErrorMessage = QStringLiteral("Failed to create weekly_budgets week index during schema migration to version 5: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to create weekly_budgets week index:" << query.lastError().text();
        return false;
    }

    const bool categoryWeekIndexCreated = query.exec(QStringLiteral(R"(
        CREATE INDEX IF NOT EXISTS idx_weekly_budgets_category_week
        ON weekly_budgets(category_id, week_start_date)
    )"));

    if (!categoryWeekIndexCreated) {
        m_lastErrorMessage = QStringLiteral("Failed to create weekly_budgets category week index during schema migration to version 5: %1")
                                 .arg(query.lastError().text());
        qWarning().noquote() << "Failed to create weekly_budgets category week index:" << query.lastError().text();
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

bool DatabaseManager::seedDefaultSubcategories()
{
    struct DefaultSubcategorySeed
    {
        QString categoryName;
        QString type;
        QStringList names;
    };

    const DefaultSubcategorySeed defaultSubcategories[] = {
        {QStringLiteral("餐饮"), QStringLiteral("expense"), {
            QStringLiteral("早餐"),
            QStringLiteral("午餐"),
            QStringLiteral("晚餐"),
            QStringLiteral("咖啡"),
            QStringLiteral("零食"),
            QStringLiteral("外卖"),
            QStringLiteral("其他"),
            QStringLiteral("请客吃饭")
        }},
        {QStringLiteral("交通"), QStringLiteral("expense"), {
            QStringLiteral("地铁"),
            QStringLiteral("公交"),
            QStringLiteral("打车"),
            QStringLiteral("火车"),
            QStringLiteral("机票"),
            QStringLiteral("停车"),
            QStringLiteral("加油"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("购物"), QStringLiteral("expense"), {
            QStringLiteral("日用品"),
            QStringLiteral("服饰"),
            QStringLiteral("电子产品"),
            QStringLiteral("网购"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("学习"), QStringLiteral("expense"), {
            QStringLiteral("书籍"),
            QStringLiteral("课程"),
            QStringLiteral("考试"),
            QStringLiteral("文具"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("娱乐"), QStringLiteral("expense"), {
            QStringLiteral("电影"),
            QStringLiteral("游戏"),
            QStringLiteral("聚会"),
            QStringLiteral("旅游"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("住房"), QStringLiteral("expense"), {
            QStringLiteral("房租"),
            QStringLiteral("水电"),
            QStringLiteral("物业"),
            QStringLiteral("维修"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("医疗"), QStringLiteral("expense"), {
            QStringLiteral("药品"),
            QStringLiteral("门诊"),
            QStringLiteral("体检"),
            QStringLiteral("保险"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("其他"), QStringLiteral("expense"), {
            QStringLiteral("其他")
        }},
        {QStringLiteral("工资"), QStringLiteral("income"), {
            QStringLiteral("基本工资"),
            QStringLiteral("奖金"),
            QStringLiteral("补贴"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("兼职"), QStringLiteral("income"), {
            QStringLiteral("项目收入"),
            QStringLiteral("小时工"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("投资"), QStringLiteral("income"), {
            QStringLiteral("股票"),
            QStringLiteral("基金"),
            QStringLiteral("利息"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("生活费"), QStringLiteral("income"), {
            QStringLiteral("家庭支持"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("奖学金"), QStringLiteral("income"), {
            QStringLiteral("学校奖学金"),
            QStringLiteral("竞赛奖金"),
            QStringLiteral("其他")
        }},
        {QStringLiteral("其他"), QStringLiteral("income"), {
            QStringLiteral("其他")
        }}
    };

    QSqlQuery categoryQuery(m_database);
    if (!categoryQuery.prepare(QStringLiteral(R"(
        SELECT id
        FROM categories
        WHERE name = :category_name
          AND type = :type
        LIMIT 1
    )"))) {
        m_lastErrorMessage = QStringLiteral("Failed to prepare default subcategory parent category SQL during schema migration to version 8: %1")
                                 .arg(categoryQuery.lastError().text());
        qWarning().noquote() << "Failed to prepare default subcategory parent category SQL:"
                             << categoryQuery.lastError().text();
        return false;
    }

    QSqlQuery insertQuery(m_database);
    if (!insertQuery.prepare(QStringLiteral(R"(
        INSERT OR IGNORE INTO subcategories (
            category_id,
            name,
            is_default,
            sort_order,
            created_at,
            updated_at
        ) VALUES (
            :category_id,
            :name,
            1,
            :sort_order,
            :created_at,
            :updated_at
        )
    )"))) {
        m_lastErrorMessage = QStringLiteral("Failed to prepare default subcategory seed SQL during schema migration to version 8: %1")
                                 .arg(insertQuery.lastError().text());
        qWarning().noquote() << "Failed to prepare default subcategory seed SQL:"
                             << insertQuery.lastError().text();
        return false;
    }

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    for (const DefaultSubcategorySeed &category : defaultSubcategories) {
        categoryQuery.bindValue(QStringLiteral(":category_name"), category.categoryName);
        categoryQuery.bindValue(QStringLiteral(":type"), category.type);

        if (!categoryQuery.exec()) {
            m_lastErrorMessage = QStringLiteral("Failed to find parent category '%1/%2' during schema migration to version 8: %3")
                                     .arg(category.type)
                                     .arg(category.categoryName)
                                     .arg(categoryQuery.lastError().text());
            qWarning().noquote() << "Failed to find parent category for default subcategories:"
                                 << category.type << category.categoryName << categoryQuery.lastError().text();
            return false;
        }

        if (!categoryQuery.next()) {
            qWarning().noquote() << "Skipped default subcategories because parent category is missing:"
                                 << category.type << category.categoryName;
            continue;
        }

        const int categoryId = categoryQuery.value(QStringLiteral("id")).toInt();
        for (int sortOrder = 0; sortOrder < category.names.size(); ++sortOrder) {
            insertQuery.bindValue(QStringLiteral(":category_id"), categoryId);
            insertQuery.bindValue(QStringLiteral(":name"), category.names.at(sortOrder));
            insertQuery.bindValue(QStringLiteral(":sort_order"), sortOrder);
            insertQuery.bindValue(QStringLiteral(":created_at"), now);
            insertQuery.bindValue(QStringLiteral(":updated_at"), now);

            if (!insertQuery.exec()) {
                m_lastErrorMessage = QStringLiteral("Failed to seed default subcategory '%1/%2' during schema migration to version 8: %3")
                                         .arg(category.categoryName)
                                         .arg(category.names.at(sortOrder))
                                         .arg(insertQuery.lastError().text());
                qWarning().noquote() << "Failed to seed default subcategory:"
                                     << category.categoryName << category.names.at(sortOrder)
                                     << insertQuery.lastError().text();
                return false;
            }
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

bool DatabaseManager::addTransactionSubcategoryColumns()
{
    bool subcategoryIdExists = false;
    if (!columnExists(QStringLiteral("transactions"), QStringLiteral("subcategory_id"), &subcategoryIdExists)) {
        return false;
    }

    QSqlQuery query(m_database);
    if (!subcategoryIdExists) {
        const bool success = query.exec(QStringLiteral(R"(
            ALTER TABLE transactions ADD COLUMN subcategory_id INTEGER
        )"));

        if (!success) {
            m_lastErrorMessage = QStringLiteral("Failed to add transactions.subcategory_id during schema migration to version 7: %1")
                                     .arg(query.lastError().text());
            qWarning().noquote() << "Failed to add transactions.subcategory_id:"
                                 << query.lastError().text();
            return false;
        }
    }

    bool subcategoryExists = false;
    if (!columnExists(QStringLiteral("transactions"), QStringLiteral("subcategory"), &subcategoryExists)) {
        return false;
    }

    if (!subcategoryExists) {
        const bool success = query.exec(QStringLiteral(R"(
            ALTER TABLE transactions ADD COLUMN subcategory TEXT
        )"));

        if (!success) {
            m_lastErrorMessage = QStringLiteral("Failed to add transactions.subcategory during schema migration to version 7: %1")
                                     .arg(query.lastError().text());
            qWarning().noquote() << "Failed to add transactions.subcategory:"
                                 << query.lastError().text();
            return false;
        }
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
