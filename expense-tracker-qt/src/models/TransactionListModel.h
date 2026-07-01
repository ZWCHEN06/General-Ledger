#pragma once

#include "Transaction.h"
#include "TransactionFilter.h"

#include <QAbstractListModel>
#include <QList>

class TransactionRepository;

class TransactionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        AmountRole,
        CategoryRole,
        DateRole,
        NoteRole,
        TransactionIdRole
    };

    explicit TransactionListModel(QObject *parent = nullptr);
    explicit TransactionListModel(TransactionRepository *transactionRepository, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    void refreshWithFilter(TransactionFilter filter);

    void setTransactionRepository(TransactionRepository *transactionRepository);
    void setTransactions(const QList<Transaction> &transactions);

private:
    TransactionRepository *m_transactionRepository = nullptr;
    QList<Transaction> m_transactions;
};
