#include "Category.h"

Category::Category() = default;

Category::Category(int id,
                   const QString &name,
                   TransactionType type,
                   bool isDefault,
                   int sortOrder,
                   const QString &createdAt,
                   const QString &updatedAt)
    : m_id(id),
      m_name(name),
      m_type(type),
      m_isDefault(isDefault),
      m_sortOrder(sortOrder),
      m_createdAt(createdAt),
      m_updatedAt(updatedAt)
{
}

bool Category::validate(QString *errorMessage) const
{
    auto setError = [errorMessage](const QString &message) {
        if (errorMessage) {
            *errorMessage = message;
        }
    };

    if (m_name.trimmed().isEmpty()) {
        setError(QStringLiteral("Category name cannot be empty"));
        return false;
    }

    switch (m_type) {
    case TransactionType::Income:
    case TransactionType::Expense:
        break;
    default:
        setError(QStringLiteral("Category type must be income or expense"));
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

int Category::id() const
{
    return m_id;
}

void Category::setId(int id)
{
    m_id = id;
}

QString Category::name() const
{
    return m_name;
}

void Category::setName(const QString &name)
{
    m_name = name;
}

TransactionType Category::type() const
{
    return m_type;
}

void Category::setType(TransactionType type)
{
    m_type = type;
}

bool Category::isDefault() const
{
    return m_isDefault;
}

void Category::setIsDefault(bool isDefault)
{
    m_isDefault = isDefault;
}

int Category::sortOrder() const
{
    return m_sortOrder;
}

void Category::setSortOrder(int sortOrder)
{
    m_sortOrder = sortOrder;
}

QString Category::createdAt() const
{
    return m_createdAt;
}

void Category::setCreatedAt(const QString &createdAt)
{
    m_createdAt = createdAt;
}

QString Category::updatedAt() const
{
    return m_updatedAt;
}

void Category::setUpdatedAt(const QString &updatedAt)
{
    m_updatedAt = updatedAt;
}
