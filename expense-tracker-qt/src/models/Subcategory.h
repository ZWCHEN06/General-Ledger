#pragma once

#include <QString>

class Subcategory
{
public:
    Subcategory();
    Subcategory(int id,
                int categoryId,
                const QString &name,
                bool isDefault,
                int sortOrder,
                const QString &createdAt,
                const QString &updatedAt);

    bool validate(QString *errorMessage = nullptr) const;

    int id() const;
    void setId(int id);

    int categoryId() const;
    void setCategoryId(int categoryId);

    QString name() const;
    void setName(const QString &name);

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
    int m_categoryId = 0;
    QString m_name;
    bool m_isDefault = false;
    int m_sortOrder = 0;
    QString m_createdAt;
    QString m_updatedAt;
};
