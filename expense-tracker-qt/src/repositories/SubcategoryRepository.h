#pragma once

#include "../models/Subcategory.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

struct SubcategoryRepositoryResult
{
    bool success = false;
    int id = 0;
    QString errorMessage;
};

class SubcategoryRepository
{
public:
    explicit SubcategoryRepository(const QSqlDatabase &database);

    QList<Subcategory> getSubcategoriesByCategoryId(int categoryId);
    QList<Subcategory> getSubcategoryById(int id);
    QList<Subcategory> findSubcategoryByNameAndCategoryId(const QString &name, int categoryId);
    SubcategoryRepositoryResult addSubcategory(int categoryId, const QString &name);

private:
    QSqlDatabase m_database;
};
