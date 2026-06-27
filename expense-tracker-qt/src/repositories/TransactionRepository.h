#pragma once

#include <QSqlDatabase>

class Transaction;

class TransactionRepository
{
public:
    explicit TransactionRepository(const QSqlDatabase &database);

    int addTransaction(const Transaction &transaction);

private:
    QSqlDatabase m_database;
};
