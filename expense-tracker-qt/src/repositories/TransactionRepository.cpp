#include "TransactionRepository.h"

TransactionRepository::TransactionRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

