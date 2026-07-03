#include "Transaction.h"

#include <QDate>

Transaction::Transaction() = default;

Transaction::Transaction(int id,
                         TransactionType type,
                         double amount,
                         const QString &category,
                         const QString &date,
                         const QString &note,
                         const QString &createdAt,
                         const QString &updatedAt,
                         std::optional<int> categoryId,
                         int subcategoryId,
                         const QString &subcategory)
    : m_id(id),
      m_type(type),
      m_amount(amount),
      m_category(category),
      m_categoryId(categoryId),
      m_subcategoryId(subcategoryId),
      m_subcategory(subcategory),
      m_date(date),
      m_note(note),
      m_createdAt(createdAt),
      m_updatedAt(updatedAt)
{
}

QString Transaction::typeToString(TransactionType type)
{
    switch (type) {
    case TransactionType::Income:
        return QStringLiteral("income");
    case TransactionType::Expense:
        return QStringLiteral("expense");
    }

    return QStringLiteral("expense");
}

TransactionType Transaction::typeFromString(const QString &type,
                                            bool *ok,
                                            TransactionType defaultValue)
{
    const QString normalizedType = type.trimmed().toLower();

    if (normalizedType == QStringLiteral("income")) {
        if (ok) {
            *ok = true;
        }
        return TransactionType::Income;
    }

    if (normalizedType == QStringLiteral("expense")) {
        if (ok) {
            *ok = true;
        }
        return TransactionType::Expense;
    }

    if (ok) {
        *ok = false;
    }
    return defaultValue;
}

bool Transaction::validate(QString *errorMessage) const
{
    auto setError = [errorMessage](const QString &message) {
        if (errorMessage) {
            *errorMessage = message;
        }
    };

    if (m_amount <= 0.0) {
        setError(QStringLiteral("金额必须大于 0"));
        return false;
    }

    switch (m_type) {
    case TransactionType::Income:
    case TransactionType::Expense:
        break;
    default:
        setError(QStringLiteral("类型必须是 income 或 expense"));
        return false;
    }

    if (m_category.trimmed().isEmpty()) {
        setError(QStringLiteral("分类不能为空"));
        return false;
    }

    const QString normalizedDate = m_date.trimmed();
    if (normalizedDate.isEmpty()) {
        setError(QStringLiteral("日期不能为空"));
        return false;
    }

    const QDate parsedDate = QDate::fromString(normalizedDate, QStringLiteral("yyyy-MM-dd"));
    if (!parsedDate.isValid() || parsedDate.toString(QStringLiteral("yyyy-MM-dd")) != normalizedDate) {
        setError(QStringLiteral("日期格式不正确，请使用 YYYY-MM-DD"));
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

int Transaction::id() const
{
    return m_id;
}

void Transaction::setId(int id)
{
    m_id = id;
}

TransactionType Transaction::type() const
{
    return m_type;
}

void Transaction::setType(TransactionType type)
{
    m_type = type;
}

double Transaction::amount() const
{
    return m_amount;
}

void Transaction::setAmount(double amount)
{
    m_amount = amount;
}

QString Transaction::category() const
{
    return m_category;
}

void Transaction::setCategory(const QString &category)
{
    m_category = category;
}

std::optional<int> Transaction::categoryId() const
{
    return m_categoryId;
}

void Transaction::setCategoryId(std::optional<int> categoryId)
{
    m_categoryId = categoryId;
}

void Transaction::setCategoryId(int categoryId)
{
    m_categoryId = categoryId;
}

void Transaction::clearCategoryId()
{
    m_categoryId = std::nullopt;
}

int Transaction::subcategoryId() const
{
    return m_subcategoryId;
}

void Transaction::setSubcategoryId(int subcategoryId)
{
    m_subcategoryId = subcategoryId;
}

QString Transaction::subcategory() const
{
    return m_subcategory;
}

void Transaction::setSubcategory(const QString &subcategory)
{
    m_subcategory = subcategory;
}

QString Transaction::date() const
{
    return m_date;
}

void Transaction::setDate(const QString &date)
{
    m_date = date;
}

QString Transaction::note() const
{
    return m_note;
}

void Transaction::setNote(const QString &note)
{
    m_note = note;
}

QString Transaction::createdAt() const
{
    return m_createdAt;
}

void Transaction::setCreatedAt(const QString &createdAt)
{
    m_createdAt = createdAt;
}

QString Transaction::updatedAt() const
{
    return m_updatedAt;
}

void Transaction::setUpdatedAt(const QString &updatedAt)
{
    m_updatedAt = updatedAt;
}
