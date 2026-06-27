#pragma once

#include <QSqlDatabase>

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool openDatabase();
    void closeDatabase();
    bool isOpen() const;
    bool initializeTables();

private:
    static constexpr const char *ConnectionName = "expense_tracker_connection";
    static constexpr const char *DatabaseFileName = "expense_tracker.db";

    QSqlDatabase m_database;
};
