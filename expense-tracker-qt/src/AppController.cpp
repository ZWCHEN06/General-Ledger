#include "AppController.h"

#include "models/Transaction.h"
#include "repositories/TransactionRepository.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

AppController::AppController(TransactionRepository *transactionRepository, QObject *parent)
    : QObject(parent),
      m_transactionRepository(transactionRepository)
{
}

QString AppController::testMessage() const
{
    return QStringLiteral("C++ 已连接");
}

QVariantMap AppController::addTransaction(const QString &type,
                                          const QString &amount,
                                          const QString &category,
                                          const QString &date,
                                          const QString &note)
{
    auto failure = [](const QString &message) {
        return QVariantMap {
            {QStringLiteral("success"), false},
            {QStringLiteral("errorMessage"), message},
            {QStringLiteral("id"), -1}
        };
    };

    if (!m_transactionRepository) {
        return failure(QStringLiteral("账单仓库未初始化"));
    }

    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(type, &typeOk);
    if (!typeOk) {
        return failure(QStringLiteral("账单类型不正确"));
    }

    bool amountOk = false;
    const double transactionAmount = amount.trimmed().toDouble(&amountOk);
    if (!amountOk) {
        return failure(QStringLiteral("金额格式不正确"));
    }

    Transaction transaction(
        0,
        transactionType,
        transactionAmount,
        category.trimmed(),
        date.trimmed(),
        note.trimmed(),
        QString(),
        QString());

    QString validationError;
    if (!transaction.validate(&validationError)) {
        return failure(validationError);
    }

    const int id = m_transactionRepository->addTransaction(transaction);
    if (id <= 0) {
        return failure(QStringLiteral("保存账单失败，请稍后重试"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), id}
    };
}
