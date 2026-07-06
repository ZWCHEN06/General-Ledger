#pragma once

#include <QList>
#include <QSqlDatabase>
#include <QString>

struct MonthlyTrendItem
{
    QString month;
    double income = 0.0;
    double expense = 0.0;
};

class ChartDataService
{
public:
    explicit ChartDataService(const QSqlDatabase &database);

    QList<MonthlyTrendItem> monthlyTrend(int months = 6) const;

private:
    QSqlDatabase m_database;
};
