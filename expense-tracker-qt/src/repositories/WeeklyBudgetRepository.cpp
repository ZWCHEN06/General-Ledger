#include "WeeklyBudgetRepository.h"

#include "../utils/WeekUtils.h"

#include <QDateTime>
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

WeeklyBudgetRepositoryResult WeeklyBudgetRepository::upsertBudget(const QString &weekStartDate,
                                                                  int categoryId,
                                                                  double amount)
{
    const QString trimmedWeekStartDate = weekStartDate.trimmed();
    const QDate parsedWeekStartDate = WeekUtils::parseDate(trimmedWeekStartDate);
    if (!parsedWeekStartDate.isValid() || !WeekUtils::isWeekStartDate(parsedWeekStartDate)) {
        return WeeklyBudgetRepositoryResult {
            false,
            0,
            QStringLiteral("周开始日期必须是合法的周一日期，格式为 YYYY-MM-DD")
        };
    }

    if (categoryId <= 0) {
        return WeeklyBudgetRepositoryResult {
            false,
            0,
            QStringLiteral("分类 id 必须大于 0")
        };
    }

    if (amount < 0.0) {
        return WeeklyBudgetRepositoryResult {
            false,
            0,
            QStringLiteral("预算金额不能小于 0")
        };
    }

    if (!m_database.isOpen()) {
        return WeeklyBudgetRepositoryResult {
            false,
            0,
            QStringLiteral("数据库未打开，无法保存每周预算")
        };
    }

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlQuery upsertQuery(m_database);
    if (!upsertQuery.prepare(QStringLiteral(R"(
        INSERT INTO weekly_budgets (
            week_start_date,
            category_id,
            amount,
            created_at,
            updated_at
        ) VALUES (
            :week_start_date,
            :category_id,
            :amount,
            :created_at,
            :updated_at
        )
        ON CONFLICT(week_start_date, category_id) DO UPDATE SET
            amount = excluded.amount,
            updated_at = excluded.updated_at
    )"))) {
        const QString error = QStringLiteral("保存每周预算失败：SQL 准备失败：%1").arg(upsertQuery.lastError().text());
        qWarning().noquote() << error;
        return WeeklyBudgetRepositoryResult {false, 0, error};
    }

    upsertQuery.bindValue(QStringLiteral(":week_start_date"), trimmedWeekStartDate);
    upsertQuery.bindValue(QStringLiteral(":category_id"), categoryId);
    upsertQuery.bindValue(QStringLiteral(":amount"), amount);
    upsertQuery.bindValue(QStringLiteral(":created_at"), now);
    upsertQuery.bindValue(QStringLiteral(":updated_at"), now);

    if (!upsertQuery.exec()) {
        const QString error = QStringLiteral("保存每周预算失败：SQL 执行失败：%1").arg(upsertQuery.lastError().text());
        qWarning().noquote() << error;
        return WeeklyBudgetRepositoryResult {false, 0, error};
    }

    QSqlQuery idQuery(m_database);
    if (!idQuery.prepare(QStringLiteral(R"(
        SELECT id
        FROM weekly_budgets
        WHERE week_start_date = :week_start_date
          AND category_id = :category_id
        LIMIT 1
    )"))) {
        const QString error = QStringLiteral("保存每周预算失败：查询预算 id 准备失败：%1").arg(idQuery.lastError().text());
        qWarning().noquote() << error;
        return WeeklyBudgetRepositoryResult {false, 0, error};
    }

    idQuery.bindValue(QStringLiteral(":week_start_date"), trimmedWeekStartDate);
    idQuery.bindValue(QStringLiteral(":category_id"), categoryId);

    if (!idQuery.exec() || !idQuery.next()) {
        const QString error = QStringLiteral("保存每周预算失败：未能获取预算 id：%1").arg(idQuery.lastError().text());
        qWarning().noquote() << error;
        return WeeklyBudgetRepositoryResult {false, 0, error};
    }

    return WeeklyBudgetRepositoryResult {
        true,
        idQuery.value(QStringLiteral("id")).toInt(),
        QString()
    };
}
