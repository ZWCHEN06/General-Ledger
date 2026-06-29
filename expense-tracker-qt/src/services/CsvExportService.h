#pragma once

#include <QList>
#include <QString>

#include "../models/Transaction.h"

class CsvExportService
{
public:
    QString exportTransactions(const QList<Transaction> &transactions) const;
};

