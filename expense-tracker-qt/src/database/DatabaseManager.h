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
    static constexpr int CurrentSchemaVersion = 6;

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
    bool createCategoriesTable();
    bool createWeeklyBudgetsTable();
    bool createSubcategoriesTable();
    bool addTransactionCategoryIdColumn();
    bool seedDefaultCategories();
    bool backfillTransactionCategoryIds();
    bool columnExists(const QString &tableName, const QString &columnName, bool *exists);

    QSqlDatabase m_database;
    QString m_lastErrorMessage;
};
