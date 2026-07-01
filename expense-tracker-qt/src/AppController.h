#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

class TransactionListModel;
class TransactionRepository;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool databaseReady READ databaseReady NOTIFY databaseStatusChanged)
    Q_PROPERTY(QString databaseErrorMessage READ databaseErrorMessage NOTIFY databaseStatusChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    explicit AppController(TransactionRepository *transactionRepository, QObject *parent = nullptr);

    bool databaseReady() const;
    QString databaseErrorMessage() const;
    void setDatabaseStatus(bool ready, const QString &errorMessage);

    Q_INVOKABLE QString testMessage() const;
    Q_INVOKABLE QVariantMap addTransaction(const QString &type,
                                           const QString &amount,
                                           const QString &category,
                                           const QString &date,
                                           const QString &note);
    Q_INVOKABLE QVariantMap currentMonthSummary() const;
    Q_INVOKABLE QVariantMap getTransactionById(int id) const;
    Q_INVOKABLE QVariantMap updateTransaction(int id,
                                              const QString &type,
                                              const QString &amount,
                                              const QString &category,
                                              const QString &date,
                                              const QString &note);
    Q_INVOKABLE QVariantMap deleteTransaction(int id);
    Q_INVOKABLE QVariantMap exportCsv() const;
    Q_INVOKABLE QVariantMap currentMonthCategorySummary() const;
    Q_INVOKABLE QVariantMap applyTransactionFilter(const QVariant &year,
                                                   const QVariant &month,
                                                   const QString &type,
                                                   const QString &category,
                                                   const QString &keyword,
                                                   const QVariant &minAmount,
                                                   const QVariant &maxAmount);
    Q_INVOKABLE QVariantMap clearTransactionFilter();

    void setTransactionListModel(TransactionListModel *transactionListModel);

signals:
    void databaseStatusChanged();

private:
    QString effectiveDatabaseErrorMessage() const;

    TransactionRepository *m_transactionRepository = nullptr;
    TransactionListModel *m_transactionListModel = nullptr;
    bool m_databaseReady = true;
    QString m_databaseErrorMessage;
};
