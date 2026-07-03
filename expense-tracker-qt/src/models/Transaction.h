#pragma once

#include <QString>
#include <optional>

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
                const QString &updatedAt,
                std::optional<int> categoryId = std::nullopt,
                int subcategoryId = -1,
                const QString &subcategory = QString());

    static QString typeToString(TransactionType type);
    static TransactionType typeFromString(const QString &type,
                                          bool *ok = nullptr,
                                          TransactionType defaultValue = TransactionType::Expense);

    bool validate(QString *errorMessage = nullptr) const;

    int id() const;
    void setId(int id);

    TransactionType type() const;
    void setType(TransactionType type);

    double amount() const;
    void setAmount(double amount);

    QString category() const;
    void setCategory(const QString &category);

    std::optional<int> categoryId() const;
    void setCategoryId(std::optional<int> categoryId);
    void setCategoryId(int categoryId);
    void clearCategoryId();

    int subcategoryId() const;
    void setSubcategoryId(int subcategoryId);

    QString subcategory() const;
    void setSubcategory(const QString &subcategory);

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
    std::optional<int> m_categoryId;
    int m_subcategoryId = -1;
    QString m_subcategory;
    QString m_date;
    QString m_note;
    QString m_createdAt;
    QString m_updatedAt;
};
