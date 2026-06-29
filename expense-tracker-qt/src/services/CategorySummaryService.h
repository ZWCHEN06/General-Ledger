#pragma once

#include <QList>
#include <QString>

#include "../models/Transaction.h"

struct CategorySummaryItem
{
    QString category;
    double amount = 0.0;
};

class CategorySummaryService
{
public:
    QList<CategorySummaryItem> calculate(const QList<Transaction> &transactions) const;
};

