#pragma once

#include <QSqlDatabase>

class TransactionRepository
{
public:
    explicit TransactionRepository(const QSqlDatabase &database);

private:
    QSqlDatabase m_database;
};

