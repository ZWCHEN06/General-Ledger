#pragma once

#include "../models/WeeklyBudget.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

class WeeklyBudgetRepository
{
public:
    explicit WeeklyBudgetRepository(const QSqlDatabase &database);

    QList<WeeklyBudget> getBudgetsByWeek(const QString &weekStartDate);

private:
    QSqlDatabase m_database;
};
