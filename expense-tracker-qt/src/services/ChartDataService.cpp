#include "ChartDataService.h"

#include <QDate>
#include <QDebug>
#include <QHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantMap>

namespace {
struct MonthlyTrendBucket
{
    int year = 0;
    int month = 0;
    double income = 0.0;
    double expense = 0.0;
};
}

ChartDataService::ChartDataService(const QSqlDatabase &database)
    : m_database(database)
{
}

QVariantList ChartDataService::monthlyTrend(int months) const
{
    QVariantList result;

    if (months <= 0) {
        qWarning().noquote() << "Failed to load monthly trend chart data: months must be greater than 0.";
        return result;
    }

    if (!m_database.isOpen()) {
        qWarning().noquote() << "Failed to load monthly trend chart data: SQLite database is not open.";
        return result;
    }

    const QDate currentMonth = QDate::currentDate().addDays(1 - QDate::currentDate().day());
    const QDate startMonth = currentMonth.addMonths(-(months - 1));
    const QDate endExclusive = currentMonth.addMonths(1);

    QList<QString> monthKeys;
    monthKeys.reserve(months);

    QHash<QString, MonthlyTrendBucket> buckets;
    buckets.reserve(months);

    for (int index = 0; index < months; ++index) {
        const QDate monthDate = startMonth.addMonths(index);
        const QString monthKey = monthDate.toString(QStringLiteral("yyyy-MM"));
        monthKeys.append(monthKey);
        buckets.insert(monthKey,
                       MonthlyTrendBucket{
                           monthDate.year(),
                           monthDate.month(),
                           0.0,
                           0.0
                       });
    }

    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        SELECT
            substr(date, 1, 7) AS year_month,
            type,
            SUM(amount) AS total_amount
        FROM transactions
        WHERE type IN (:incomeType, :expenseType)
          AND date >= :startDate
          AND date < :endDate
        GROUP BY year_month, type
        ORDER BY year_month ASC
    )"))) {
        qWarning().noquote() << "Failed to prepare monthly trend chart query:"
                             << query.lastError().text();
        return result;
    }

    query.bindValue(QStringLiteral(":incomeType"), QStringLiteral("income"));
    query.bindValue(QStringLiteral(":expenseType"), QStringLiteral("expense"));
    query.bindValue(QStringLiteral(":startDate"), startMonth.toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":endDate"), endExclusive.toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning().noquote() << "Failed to execute monthly trend chart query:"
                             << query.lastError().text();
        return result;
    }

    while (query.next()) {
        const QString monthKey = query.value(QStringLiteral("year_month")).toString();
        if (!buckets.contains(monthKey)) {
            continue;
        }

        MonthlyTrendBucket bucket = buckets.value(monthKey);
        const QString type = query.value(QStringLiteral("type")).toString();
        const double amount = query.value(QStringLiteral("total_amount")).toDouble();

        if (type == QStringLiteral("income")) {
            bucket.income = amount;
        } else if (type == QStringLiteral("expense")) {
            bucket.expense = amount;
        }

        buckets.insert(monthKey, bucket);
    }

    result.reserve(monthKeys.size());
    for (const QString &monthKey : monthKeys) {
        const MonthlyTrendBucket bucket = buckets.value(monthKey);

        QVariantMap item;
        item.insert(QStringLiteral("year"), bucket.year);
        item.insert(QStringLiteral("month"), bucket.month);
        item.insert(QStringLiteral("label"), QStringLiteral("%1月").arg(bucket.month));
        item.insert(QStringLiteral("income"), bucket.income);
        item.insert(QStringLiteral("expense"), bucket.expense);
        result.append(item);
    }

    return result;
}
