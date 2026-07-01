#pragma once

#include "Transaction.h"

#include <QString>

#include <optional>

struct TransactionFilter
{
    bool validate(QString *errorMessage = nullptr) const;
    bool hasActiveConditions() const;

    std::optional<int> year;
    std::optional<int> month;
    std::optional<TransactionType> type;
    std::optional<QString> category;
    std::optional<QString> keyword;
    std::optional<double> minAmount;
    std::optional<double> maxAmount;
};
