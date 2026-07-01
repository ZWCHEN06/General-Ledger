#include "CategoryRepository.h"

CategoryRepository::CategoryRepository(const QSqlDatabase &database)
    : m_database(database)
{
}
