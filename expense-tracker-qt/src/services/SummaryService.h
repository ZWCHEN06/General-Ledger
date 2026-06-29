#pragma once

#include <QList>

#include "../models/Transaction.h"

struct SummaryResult
{
    double totalIncome = 0.0;
    double totalExpense = 0.0;
    double balance = 0.0;
};

class SummaryService
{
public:
    SummaryResult calculate(const QList<Transaction> &transactions) const;
};

