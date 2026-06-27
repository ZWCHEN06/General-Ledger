#pragma once

#include <QString>

enum class TransactionType {
    Income,
    Expense
};

class Transaction
{
public:
    Transaction();
    Transaction(int id,
                TransactionType type,
                double amount,
                const QString &category,
                const QString &date,
                const QString &note,
                const QString &createdAt,
                const QString &updatedAt);

    static QString typeToString(TransactionType type);
    static TransactionType typeFromString(const QString &type,
                                          bool *ok = nullptr,
                                          TransactionType defaultValue = TransactionType::Expense);

    int id() const;
    void setId(int id);

    TransactionType type() const;
    void setType(TransactionType type);

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
    TransactionType m_type = TransactionType::Expense;
    double m_amount = 0.0;
    QString m_category;
    QString m_date;
    QString m_note;
    QString m_createdAt;
    QString m_updatedAt;
};
