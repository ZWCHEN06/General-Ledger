#include "TransactionListModel.h"

#include "../repositories/TransactionRepository.h"

TransactionListModel::TransactionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TransactionListModel::TransactionListModel(TransactionRepository *transactionRepository, QObject *parent)
    : QAbstractListModel(parent),
      m_transactionRepository(transactionRepository)
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
    case TransactionIdRole:
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
        {NoteRole, "note"},
        {TransactionIdRole, "transactionId"}
    };
}

void TransactionListModel::refresh()
{
    if (!m_transactionRepository) {
        setTransactions({});
        return;
    }

    setTransactions(m_transactionRepository->getAllTransactions());
}

void TransactionListModel::refreshWithFilter(TransactionFilter filter)
{
    if (!m_transactionRepository) {
        setTransactions({});
        return;
    }

    setTransactions(m_transactionRepository->getTransactionsByFilter(filter));
}

void TransactionListModel::setTransactionRepository(TransactionRepository *transactionRepository)
{
    m_transactionRepository = transactionRepository;
}

void TransactionListModel::setTransactions(const QList<Transaction> &transactions)
{
    beginResetModel();
    m_transactions = transactions;
    endResetModel();
}
