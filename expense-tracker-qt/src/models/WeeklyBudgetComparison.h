#pragma once

#include <QString>

struct WeeklyBudgetComparisonItem
{
    int categoryId = 0;
    QString categoryName;
    double budgetAmount = 0.0;
    double actualAmount = 0.0;
    double remainingAmount = 0.0;
    double usagePercent = 0.0;
    bool isOverBudget = false;
    bool hasBudget = false;
};

struct WeeklyBudgetSummary
{
    double totalBudget = 0.0;
    double totalActual = 0.0;
    double totalRemaining = 0.0;
    double totalUsagePercent = 0.0;
    bool isTotalOverBudget = false;
};
