#pragma once

#include <QList>
#include <QSqlDatabase>

#include <optional>

#include "../models/Transaction.h"

class TransactionRepository
{
public:
    explicit TransactionRepository(const QSqlDatabase &database);

    int addTransaction(const Transaction &transaction);
    QList<Transaction> getAllTransactions();
    std::optional<Transaction> getTransactionById(int id);

private:
    QSqlDatabase m_database;
};
