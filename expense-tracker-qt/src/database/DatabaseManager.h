#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool openDatabase();
    void closeDatabase();
    bool isOpen() const;
    bool initializeTables();
    QString lastErrorMessage() const;
    QSqlDatabase database() const;

private:
    static constexpr const char *ConnectionName = "expense_tracker_connection";
    static constexpr const char *DatabaseFileName = "expense_tracker.db";
    static constexpr int CurrentSchemaVersion = 9;

    bool enableForeignKeys();
    bool ensureTransactionsTable();
    int currentUserVersion();
    bool setUserVersion(int version);
    bool migrateDatabase();
    bool migrateToVersion1();
    bool migrateToVersion2();
    bool migrateToVersion3();
    bool migrateToVersion4();
    bool migrateToVersion5();
    bool migrateToVersion6();
    bool migrateToVersion7();
    bool migrateToVersion8();
    bool migrateToVersion9();
    bool createCategoriesTable();
    bool createWeeklyBudgetsTable();
    bool createSubcategoriesTable();
    bool createSubcategoryIndexes();
    bool addTransactionCategoryIdColumn();
    bool addTransactionSubcategoryColumns();
    bool seedDefaultCategories();
    bool seedDefaultSubcategories();
    bool backfillTransactionCategoryIds();
    bool columnExists(const QString &tableName, const QString &columnName, bool *exists);

    QSqlDatabase m_database;
    QString m_lastErrorMessage;
};
