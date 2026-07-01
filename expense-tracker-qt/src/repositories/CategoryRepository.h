#pragma once

#include <QSqlDatabase>

class CategoryRepository
{
public:
    explicit CategoryRepository(const QSqlDatabase &database);

private:
    QSqlDatabase m_database;
};
