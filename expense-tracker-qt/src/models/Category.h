#pragma once

#include "Transaction.h"

#include <QString>

class Category
{
public:
    Category();
    Category(int id,
             const QString &name,
             TransactionType type,
             bool isDefault,
             int sortOrder,
             const QString &createdAt,
             const QString &updatedAt);

    bool validate(QString *errorMessage = nullptr) const;

    int id() const;
    void setId(int id);

    QString name() const;
    void setName(const QString &name);

    TransactionType type() const;
    void setType(TransactionType type);

    bool isDefault() const;
    void setIsDefault(bool isDefault);

    int sortOrder() const;
    void setSortOrder(int sortOrder);

    QString createdAt() const;
    void setCreatedAt(const QString &createdAt);

    QString updatedAt() const;
    void setUpdatedAt(const QString &updatedAt);

private:
    int m_id = 0;
    QString m_name;
    TransactionType m_type = TransactionType::Expense;
    bool m_isDefault = false;
    int m_sortOrder = 0;
    QString m_createdAt;
    QString m_updatedAt;
};
