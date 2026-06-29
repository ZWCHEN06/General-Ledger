#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class TransactionRepository;

class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    explicit AppController(TransactionRepository *transactionRepository, QObject *parent = nullptr);

    Q_INVOKABLE QString testMessage() const;
    Q_INVOKABLE QVariantMap addTransaction(const QString &type,
                                           const QString &amount,
                                           const QString &category,
                                           const QString &date,
                                           const QString &note);

private:
    TransactionRepository *m_transactionRepository = nullptr;
};
