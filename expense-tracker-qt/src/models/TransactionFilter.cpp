#include "TransactionFilter.h"

namespace {
void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage) {
        *errorMessage = message;
    }
}
}

bool TransactionFilter::validate(QString *errorMessage) const
{
    if (year.has_value() && (year.value() < 1 || year.value() > 9999)) {
        setError(errorMessage, QStringLiteral("年份必须在 1 到 9999 之间"));
        return false;
    }

    if (month.has_value() && (month.value() < 1 || month.value() > 12)) {
        setError(errorMessage, QStringLiteral("月份必须在 1 到 12 之间"));
        return false;
    }

    if (minAmount.has_value() && minAmount.value() < 0.0) {
        setError(errorMessage, QStringLiteral("最小金额不能小于 0"));
        return false;
    }

    if (maxAmount.has_value() && maxAmount.value() < 0.0) {
        setError(errorMessage, QStringLiteral("最大金额不能小于 0"));
        return false;
    }

    if (minAmount.has_value() && maxAmount.has_value() && minAmount.value() > maxAmount.value()) {
        setError(errorMessage, QStringLiteral("最小金额不能大于最大金额"));
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

bool TransactionFilter::hasActiveConditions() const
{
    return year.has_value()
        || month.has_value()
        || type.has_value()
        || !category.value_or(QString()).trimmed().isEmpty()
        || !keyword.value_or(QString()).trimmed().isEmpty()
        || minAmount.has_value()
        || maxAmount.has_value();
}
