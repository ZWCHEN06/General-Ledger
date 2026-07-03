#include "Subcategory.h"

Subcategory::Subcategory() = default;

Subcategory::Subcategory(int id,
                         int categoryId,
                         const QString &name,
                         bool isDefault,
                         int sortOrder,
                         const QString &createdAt,
                         const QString &updatedAt)
    : m_id(id),
      m_categoryId(categoryId),
      m_name(name),
      m_isDefault(isDefault),
      m_sortOrder(sortOrder),
      m_createdAt(createdAt),
      m_updatedAt(updatedAt)
{
}

bool Subcategory::validate(QString *errorMessage) const
{
    auto setError = [errorMessage](const QString &message) {
        if (errorMessage) {
            *errorMessage = message;
        }
    };

    if (m_categoryId <= 0) {
        setError(QStringLiteral("一级分类ID必须大于0"));
        return false;
    }

    if (m_name.trimmed().isEmpty()) {
        setError(QStringLiteral("二级分类名称不能为空"));
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

int Subcategory::id() const
{
    return m_id;
}

void Subcategory::setId(int id)
{
    m_id = id;
}

int Subcategory::categoryId() const
{
    return m_categoryId;
}

void Subcategory::setCategoryId(int categoryId)
{
    m_categoryId = categoryId;
}

QString Subcategory::name() const
{
    return m_name;
}

void Subcategory::setName(const QString &name)
{
    m_name = name;
}

bool Subcategory::isDefault() const
{
    return m_isDefault;
}

void Subcategory::setIsDefault(bool isDefault)
{
    m_isDefault = isDefault;
}

int Subcategory::sortOrder() const
{
    return m_sortOrder;
}

void Subcategory::setSortOrder(int sortOrder)
{
    m_sortOrder = sortOrder;
}

QString Subcategory::createdAt() const
{
    return m_createdAt;
}

void Subcategory::setCreatedAt(const QString &createdAt)
{
    m_createdAt = createdAt;
}

QString Subcategory::updatedAt() const
{
    return m_updatedAt;
}

void Subcategory::setUpdatedAt(const QString &updatedAt)
{
    m_updatedAt = updatedAt;
}
