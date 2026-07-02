#include "WeeklyBudgetRepository.h"

#include "../utils/WeekUtils.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

namespace {
WeeklyBudget weeklyBudgetFromCurrentRow(const QSqlQuery &query)
{
    return WeeklyBudget(
        query.value(QStringLiteral("id")).toInt(),
        query.value(QStringLiteral("week_start_date")).toString(),
        query.value(QStringLiteral("category_id")).toInt(),
        query.value(QStringLiteral("amount")).toDouble(),
        query.value(QStringLiteral("created_at")).toString(),
        query.value(QStringLiteral("updated_at")).toString());
}
}

WeeklyBudgetRepository::WeeklyBudgetRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

QList<WeeklyBudget> WeeklyBudgetRepository::getBudgetsByWeek(const QString &weekStartDate)
{
    QList<WeeklyBudget> budgets;

    const QString trimmedWeekStartDate = weekStartDate.trimmed();
    const QDate parsedWeekStartDate = WeekUtils::parseDate(trimmedWeekStartDate);
    if (!parsedWeekStartDate.isValid() || !WeekUtils::isWeekStartDate(parsedWeekStartDate)) {
        qWarning().noquote() << "查询每周预算失败：weekStartDate 必须是合法的周一日期，格式为 YYYY-MM-DD:"
                             << weekStartDate;
        return budgets;
    }

    if (!m_database.isOpen()) {
        qWarning().noquote() << "查询每周预算失败：数据库未打开";
        return budgets;
    }

    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        SELECT
            id,
            week_start_date,
            category_id,
            amount,
            created_at,
            updated_at
        FROM weekly_budgets
        WHERE week_start_date = :week_start_date
        ORDER BY category_id ASC
    )"))) {
        qWarning().noquote() << "查询每周预算失败：SQL 准备失败:" << query.lastError().text();
        return budgets;
    }

    query.bindValue(QStringLiteral(":week_start_date"), trimmedWeekStartDate);

    if (!query.exec()) {
        qWarning().noquote() << "查询每周预算失败：SQL 执行失败:" << query.lastError().text();
        return budgets;
    }

    while (query.next()) {
        budgets.append(weeklyBudgetFromCurrentRow(query));
    }

    return budgets;
}
