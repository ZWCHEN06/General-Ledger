#pragma once

#include <QSqlDatabase>
#include <QVariantList>

class ChartDataService
{
public:
    explicit ChartDataService(const QSqlDatabase &database);

    QVariantList monthlyTrend(int months = 6) const;

private:
    QSqlDatabase m_database;
};
