#pragma once

#include <QHash>
#include <QList>
#include <QSqlDatabase>
#include <QString>

#include <optional>

#include "../models/Transaction.h"

struct TransactionFilter;

struct ExpenseCategorySummaryItem
{
    int categoryId = -1;
    QString category;
    double amount = 0.0;
};

class TransactionRepository
{
public:
    explicit TransactionRepository(const QSqlDatabase &database);

    int addTransaction(const Transaction &transaction);
    QList<Transaction> getAllTransactions();
    QList<Transaction> getTransactionsByFilter(const TransactionFilter &filter);
    QList<Transaction> getTransactionsByMonth(int year, int month);
    QList<ExpenseCategorySummaryItem> getMonthlyExpenseByCategory(int year, int month);
    QHash<int, double> getWeeklyExpenseByCategory(const QString &weekStartDate, const QString &weekEndDate);
    std::optional<Transaction> getTransactionById(int id);
    bool updateTransaction(const Transaction &transaction);
    bool deleteTransaction(int id);

private:
    QSqlDatabase m_database;
};
