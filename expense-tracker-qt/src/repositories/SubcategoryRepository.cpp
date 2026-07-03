#include "SubcategoryRepository.h"

SubcategoryRepository::SubcategoryRepository(const QSqlDatabase &database)
    : m_database(database)
{
}
