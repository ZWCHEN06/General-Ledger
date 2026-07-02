#pragma once

#include "../models/Category.h"
#include "../models/WeeklyBudget.h"
#include "../models/WeeklyBudgetComparison.h"

#include <QHash>
#include <QList>

class WeeklyBudgetService
{
public:
    QList<WeeklyBudgetComparisonItem> calculate(const QList<Category> &categories,
                                                const QList<WeeklyBudget> &budgets,
                                                const QHash<int, double> &actualExpensesByCategory) const;
};
