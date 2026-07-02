#pragma once

#include "../models/Category.h"
#include "../models/Transaction.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

struct CategoryRepositoryResult
{
    bool success = false;
    int id = 0;
    QString errorMessage;
};

class CategoryRepository
{
public:
    explicit CategoryRepository(const QSqlDatabase &database);

    QList<Category> getCategoriesByType(TransactionType type);
    CategoryRepositoryResult addCategory(const QString &name, TransactionType type);
    CategoryRepositoryResult updateCategoryName(int id, const QString &name);
    CategoryRepositoryResult deleteCategory(int id);

private:
    QSqlDatabase m_database;
};
