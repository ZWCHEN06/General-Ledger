#pragma once

#include <QSqlDatabase>

class WeeklyBudgetRepository
{
public:
    explicit WeeklyBudgetRepository(const QSqlDatabase &database);

private:
    QSqlDatabase m_database;
};
