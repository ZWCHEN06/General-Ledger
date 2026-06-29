#include "TransactionListModel.h"

TransactionListModel::TransactionListModel(QObject *parent)
    : QAbstractListModel(parent),
      m_transactions({
          Transaction(1,
                      TransactionType::Expense,
                      35.50,
                      QStringLiteral("餐饮"),
                      QStringLiteral("2026-06-29"),
                      QStringLiteral("午餐"),
                      QStringLiteral("2026-06-29T12:00:00"),
                      QStringLiteral("2026-06-29T12:00:00")),
          Transaction(2,
                      TransactionType::Income,
                      5000.00,
                      QStringLiteral("工资"),
                      QStringLiteral("2026-06-28"),
                      QStringLiteral("六月工资"),
                      QStringLiteral("2026-06-28T09:00:00"),
                      QStringLiteral("2026-06-28T09:00:00")),
          Transaction(3,
                      TransactionType::Expense,
                      128.00,
                      QStringLiteral("交通"),
                      QStringLiteral("2026-06-27"),
                      QStringLiteral("高铁"),
                      QStringLiteral("2026-06-27T18:30:00"),
                      QStringLiteral("2026-06-27T18:30:00"))
      })
{
}

int TransactionListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_transactions.size();
}

QVariant TransactionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_transactions.size()) {
        return {};
    }

    const Transaction &transaction = m_transactions.at(index.row());

    switch (role) {
    case IdRole:
        return transaction.id();
    case TypeRole:
        return Transaction::typeToString(transaction.type());
    case AmountRole:
        return transaction.amount();
    case CategoryRole:
        return transaction.category();
    case DateRole:
        return transaction.date();
    case NoteRole:
        return transaction.note();
    default:
        return {};
    }
}

QHash<int, QByteArray> TransactionListModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {TypeRole, "type"},
        {AmountRole, "amount"},
        {CategoryRole, "category"},
        {DateRole, "date"},
        {NoteRole, "note"}
    };
}

void TransactionListModel::setTransactions(const QList<Transaction> &transactions)
{
    beginResetModel();
    m_transactions = transactions;
    endResetModel();
}

