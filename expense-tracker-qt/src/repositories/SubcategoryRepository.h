#pragma once

#include <QSqlDatabase>

class SubcategoryRepository
{
public:
    explicit SubcategoryRepository(const QSqlDatabase &database);

private:
    QSqlDatabase m_database;
};
