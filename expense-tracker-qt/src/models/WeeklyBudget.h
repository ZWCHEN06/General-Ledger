#pragma once

#include <QString>

class WeeklyBudget
{
public:
    WeeklyBudget();
    WeeklyBudget(int id,
                 const QString &weekStartDate,
                 int categoryId,
                 double amount,
                 const QString &createdAt,
                 const QString &updatedAt);

    bool validate(QString *errorMessage = nullptr) const;

    int id() const;
    void setId(int id);

    QString weekStartDate() const;
    void setWeekStartDate(const QString &weekStartDate);

    int categoryId() const;
    void setCategoryId(int categoryId);

    double amount() const;
    void setAmount(double amount);

    QString createdAt() const;
    void setCreatedAt(const QString &createdAt);

    QString updatedAt() const;
    void setUpdatedAt(const QString &updatedAt);

private:
    int m_id = 0;
    QString m_weekStartDate;
    int m_categoryId = 0;
    double m_amount = 0.0;
    QString m_createdAt;
    QString m_updatedAt;
};
