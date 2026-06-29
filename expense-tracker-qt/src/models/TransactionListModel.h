#pragma once

#include "Transaction.h"

#include <QAbstractListModel>
#include <QList>

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
        NoteRole
    };

    explicit TransactionListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTransactions(const QList<Transaction> &transactions);

private:
    QList<Transaction> m_transactions;
};

