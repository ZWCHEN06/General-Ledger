#pragma once

#include "../models/WeeklyBudget.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

struct WeeklyBudgetRepositoryResult
{
    bool success = false;
    int id = 0;
    QString errorMessage;
};

class WeeklyBudgetRepository
{
public:
    explicit WeeklyBudgetRepository(const QSqlDatabase &database);

    QList<WeeklyBudget> getBudgetsByWeek(const QString &weekStartDate);
    WeeklyBudgetRepositoryResult upsertBudget(const QString &weekStartDate, int categoryId, double amount);

private:
    QSqlDatabase m_database;
};
