#include "Transaction.h"

Transaction::Transaction() = default;

Transaction::Transaction(int id,
                         Type type,
                         double amount,
                         const QString &category,
                         const QString &date,
                         const QString &note,
                         const QString &createdAt,
                         const QString &updatedAt)
    : m_id(id),
      m_type(type),
      m_amount(amount),
      m_category(category),
      m_date(date),
      m_note(note),
      m_createdAt(createdAt),
      m_updatedAt(updatedAt)
{
}

int Transaction::id() const
{
    return m_id;
}

void Transaction::setId(int id)
{
    m_id = id;
}

Transaction::Type Transaction::type() const
{
    return m_type;
}

void Transaction::setType(Type type)
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

