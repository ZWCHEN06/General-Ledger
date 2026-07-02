#pragma once

#include "WeeklyBudgetComparison.h"

#include <QAbstractListModel>
#include <QList>

class WeeklyBudgetListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        CategoryIdRole = Qt::UserRole + 1,
        CategoryNameRole,
        BudgetAmountRole,
        ActualAmountRole,
        RemainingAmountRole,
        UsagePercentRole,
        IsOverBudgetRole,
        HasBudgetRole
    };

    explicit WeeklyBudgetListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QList<WeeklyBudgetComparisonItem> &items);
    Q_INVOKABLE void clear();

private:
    QList<WeeklyBudgetComparisonItem> m_items;
};
