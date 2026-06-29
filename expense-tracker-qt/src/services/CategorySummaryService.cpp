#include "CategorySummaryService.h"

#include <QHash>
#include <algorithm>

QList<CategorySummaryItem> CategorySummaryService::calculate(const QList<Transaction> &transactions) const
{
    QHash<QString, double> amountsByCategory;

    for (const Transaction &transaction : transactions) {
        if (transaction.type() != TransactionType::Expense) {
            continue;
        }

        amountsByCategory[transaction.category()] += transaction.amount();
    }

    QList<CategorySummaryItem> result;
    result.reserve(amountsByCategory.size());

    for (auto it = amountsByCategory.cbegin(); it != amountsByCategory.cend(); ++it) {
        result.append(CategorySummaryItem{it.key(), it.value()});
    }

    std::sort(result.begin(), result.end(), [](const CategorySummaryItem &left, const CategorySummaryItem &right) {
        return left.amount > right.amount;
    });

    return result;
}

