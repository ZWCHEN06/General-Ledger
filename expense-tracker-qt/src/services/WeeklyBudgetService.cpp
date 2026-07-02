#include "WeeklyBudgetService.h"

QList<WeeklyBudgetComparisonItem> WeeklyBudgetService::calculate(
    const QList<Category> &categories,
    const QList<WeeklyBudget> &budgets,
    const QHash<int, double> &actualExpensesByCategory) const
{
    QHash<int, WeeklyBudget> budgetsByCategory;
    budgetsByCategory.reserve(budgets.size());

    for (const WeeklyBudget &budget : budgets) {
        budgetsByCategory.insert(budget.categoryId(), budget);
    }

    QList<WeeklyBudgetComparisonItem> result;
    result.reserve(categories.size());

    for (const Category &category : categories) {
        if (category.type() != TransactionType::Expense) {
            continue;
        }

        WeeklyBudgetComparisonItem item;
        item.categoryId = category.id();
        item.categoryName = category.name();
        item.actualAmount = actualExpensesByCategory.value(category.id(), 0.0);

        const auto budgetIt = budgetsByCategory.constFind(category.id());
        if (budgetIt != budgetsByCategory.cend()) {
            item.hasBudget = true;
            item.budgetAmount = budgetIt.value().amount();
            item.remainingAmount = item.budgetAmount - item.actualAmount;
            item.isOverBudget = item.actualAmount > item.budgetAmount;
            if (item.budgetAmount > 0.0) {
                item.usagePercent = item.actualAmount / item.budgetAmount * 100.0;
            }
        } else {
            item.hasBudget = false;
            item.budgetAmount = 0.0;
            item.remainingAmount = 0.0 - item.actualAmount;
            item.isOverBudget = false;
            item.usagePercent = 0.0;
        }

        result.append(item);
    }

    return result;
}

WeeklyBudgetSummary WeeklyBudgetService::summarize(const QList<WeeklyBudgetComparisonItem> &items) const
{
    WeeklyBudgetSummary summary;

    for (const WeeklyBudgetComparisonItem &item : items) {
        if (item.hasBudget) {
            summary.totalBudget += item.budgetAmount;
        }

        summary.totalActual += item.actualAmount;
    }

    summary.totalRemaining = summary.totalBudget - summary.totalActual;
    summary.isTotalOverBudget = summary.totalActual > summary.totalBudget;

    if (summary.totalBudget > 0.0) {
        summary.totalUsagePercent = summary.totalActual / summary.totalBudget * 100.0;
    }

    return summary;
}
