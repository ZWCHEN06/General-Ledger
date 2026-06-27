#pragma once

#include <QString>

class Transaction
{
public:
    enum class Type {
        Income,
        Expense
    };

    Transaction();
    Transaction(int id,
                Type type,
                double amount,
                const QString &category,
                const QString &date,
                const QString &note,
                const QString &createdAt,
                const QString &updatedAt);

    int id() const;
    void setId(int id);

    Type type() const;
    void setType(Type type);

    double amount() const;
    void setAmount(double amount);

    QString category() const;
    void setCategory(const QString &category);

    QString date() const;
    void setDate(const QString &date);

    QString note() const;
    void setNote(const QString &note);

    QString createdAt() const;
    void setCreatedAt(const QString &createdAt);

    QString updatedAt() const;
    void setUpdatedAt(const QString &updatedAt);

private:
    int m_id = 0;
    Type m_type = Type::Expense;
    double m_amount = 0.0;
    QString m_category;
    QString m_date;
    QString m_note;
    QString m_createdAt;
    QString m_updatedAt;
};

