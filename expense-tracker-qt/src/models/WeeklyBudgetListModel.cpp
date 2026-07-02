#include "WeeklyBudgetListModel.h"

WeeklyBudgetListModel::WeeklyBudgetListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WeeklyBudgetListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_items.size();
}

QVariant WeeklyBudgetListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }

    const WeeklyBudgetComparisonItem &item = m_items.at(index.row());

    switch (role) {
    case CategoryIdRole:
        return item.categoryId;
    case CategoryNameRole:
        return item.categoryName;
    case BudgetAmountRole:
        return item.budgetAmount;
    case ActualAmountRole:
        return item.actualAmount;
    case RemainingAmountRole:
        return item.remainingAmount;
    case UsagePercentRole:
        return item.usagePercent;
    case IsOverBudgetRole:
        return item.isOverBudget;
    case HasBudgetRole:
        return item.hasBudget;
    default:
        return {};
    }
}

QHash<int, QByteArray> WeeklyBudgetListModel::roleNames() const
{
    return {
        {CategoryIdRole, "categoryId"},
        {CategoryNameRole, "categoryName"},
        {BudgetAmountRole, "budgetAmount"},
        {ActualAmountRole, "actualAmount"},
        {RemainingAmountRole, "remainingAmount"},
        {UsagePercentRole, "usagePercent"},
        {IsOverBudgetRole, "isOverBudget"},
        {HasBudgetRole, "hasBudget"}
    };
}

void WeeklyBudgetListModel::setItems(const QList<WeeklyBudgetComparisonItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void WeeklyBudgetListModel::clear()
{
    setItems({});
}
