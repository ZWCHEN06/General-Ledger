#pragma once

#include "../models/Transaction.h"

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

    CategoryRepositoryResult addCategory(const QString &name, TransactionType type);
    CategoryRepositoryResult updateCategoryName(int id, const QString &name);

private:
    QSqlDatabase m_database;
};
