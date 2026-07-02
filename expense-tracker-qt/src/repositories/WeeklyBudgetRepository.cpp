#include "WeeklyBudgetRepository.h"

WeeklyBudgetRepository::WeeklyBudgetRepository(const QSqlDatabase &database)
    : m_database(database)
{
}
