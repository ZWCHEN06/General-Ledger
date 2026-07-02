#include "WeeklyBudget.h"

#include "../utils/WeekUtils.h"

WeeklyBudget::WeeklyBudget() = default;

WeeklyBudget::WeeklyBudget(int id,
                           const QString &weekStartDate,
                           int categoryId,
                           double amount,
                           const QString &createdAt,
                           const QString &updatedAt)
    : m_id(id),
      m_weekStartDate(weekStartDate),
      m_categoryId(categoryId),
      m_amount(amount),
      m_createdAt(createdAt),
      m_updatedAt(updatedAt)
{
}

bool WeeklyBudget::validate(QString *errorMessage) const
{
    auto setError = [errorMessage](const QString &message) {
        if (errorMessage) {
            *errorMessage = message;
        }
    };

    const QString trimmedWeekStartDate = m_weekStartDate.trimmed();
    if (trimmedWeekStartDate.isEmpty()) {
        setError(QStringLiteral("周开始日期不能为空"));
        return false;
    }

    const QDate parsedWeekStartDate = WeekUtils::parseDate(trimmedWeekStartDate);
    if (!parsedWeekStartDate.isValid()) {
        setError(QStringLiteral("周开始日期必须是合法的 YYYY-MM-DD 格式"));
        return false;
    }

    if (!WeekUtils::isWeekStartDate(parsedWeekStartDate)) {
        setError(QStringLiteral("周开始日期必须是周一"));
        return false;
    }

    if (m_categoryId <= 0) {
        setError(QStringLiteral("分类 id 必须大于 0"));
        return false;
    }

    if (m_amount < 0.0) {
        setError(QStringLiteral("预算金额不能小于 0"));
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

int WeeklyBudget::id() const
{
    return m_id;
}

void WeeklyBudget::setId(int id)
{
    m_id = id;
}

QString WeeklyBudget::weekStartDate() const
{
    return m_weekStartDate;
}

void WeeklyBudget::setWeekStartDate(const QString &weekStartDate)
{
    m_weekStartDate = weekStartDate;
}

int WeeklyBudget::categoryId() const
{
    return m_categoryId;
}

void WeeklyBudget::setCategoryId(int categoryId)
{
    m_categoryId = categoryId;
}

double WeeklyBudget::amount() const
{
    return m_amount;
}

void WeeklyBudget::setAmount(double amount)
{
    m_amount = amount;
}

QString WeeklyBudget::createdAt() const
{
    return m_createdAt;
}

void WeeklyBudget::setCreatedAt(const QString &createdAt)
{
    m_createdAt = createdAt;
}

QString WeeklyBudget::updatedAt() const
{
    return m_updatedAt;
}

void WeeklyBudget::setUpdatedAt(const QString &updatedAt)
{
    m_updatedAt = updatedAt;
}
