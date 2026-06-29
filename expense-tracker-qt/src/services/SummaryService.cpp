#include "SummaryService.h"

SummaryResult SummaryService::calculate(const QList<Transaction> &transactions) const
{
    SummaryResult result;

    for (const Transaction &transaction : transactions) {
        switch (transaction.type()) {
        case TransactionType::Income:
            result.totalIncome += transaction.amount();
            break;
        case TransactionType::Expense:
            result.totalExpense += transaction.amount();
            break;
        }
    }

    result.balance = result.totalIncome - result.totalExpense;
    return result;
}

