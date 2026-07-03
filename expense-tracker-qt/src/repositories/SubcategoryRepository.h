#pragma once

#include "../models/Subcategory.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

class SubcategoryRepository
{
public:
    explicit SubcategoryRepository(const QSqlDatabase &database);

    QList<Subcategory> getSubcategoriesByCategoryId(int categoryId);
    QList<Subcategory> getSubcategoryById(int id);
    QList<Subcategory> findSubcategoryByNameAndCategoryId(const QString &name, int categoryId);

private:
    QSqlDatabase m_database;
};
