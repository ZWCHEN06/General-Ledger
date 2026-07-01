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
    QSqlDatabase database() const;

private:
    static constexpr const char *ConnectionName = "expense_tracker_connection";
    static constexpr const char *DatabaseFileName = "expense_tracker.db";
    static constexpr int CurrentSchemaVersion = 1;

    bool ensureTransactionsTable();
    int currentUserVersion() const;
    bool setUserVersion(int version);
    bool migrateDatabase();
    bool migrateToVersion1();
    bool createCategoriesTable();
    bool addTransactionCategoryIdColumn();
    bool seedDefaultCategories(const QString &timestamp);
    bool backfillCategoriesFromTransactions(const QString &timestamp);
    bool backfillTransactionCategoryIds();
    bool columnExists(const QString &tableName, const QString &columnName) const;

    QSqlDatabase m_database;
};
