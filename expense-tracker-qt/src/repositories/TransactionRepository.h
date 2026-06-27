#pragma once

#include <QList>
#include <QSqlDatabase>

#include "../models/Transaction.h"

class TransactionRepository
{
public:
    explicit TransactionRepository(const QSqlDatabase &database);

    int addTransaction(const Transaction &transaction);
    QList<Transaction> getAllTransactions();

private:
    QSqlDatabase m_database;
};
