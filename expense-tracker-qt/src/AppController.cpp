#include "AppController.h"

#include "models/Transaction.h"
#include "repositories/TransactionRepository.h"
#include "services/SummaryService.h"

#include <QDate>

namespace {
QVariantMap failureResult(const QString &message)
{
    return QVariantMap {
        {QStringLiteral("success"), false},
        {QStringLiteral("errorMessage"), message},
        {QStringLiteral("id"), -1}
    };
}

QVariantMap transactionToMap(const Transaction &transaction)
{
    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), transaction.id()},
        {QStringLiteral("type"), Transaction::typeToString(transaction.type())},
        {QStringLiteral("amount"), transaction.amount()},
        {QStringLiteral("category"), transaction.category()},
        {QStringLiteral("date"), transaction.date()},
        {QStringLiteral("note"), transaction.note()}
    };
}
}

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
    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(type, &typeOk);
    if (!typeOk) {
        return failureResult(QStringLiteral("账单类型不正确"));
    }

    bool amountOk = false;
    const double transactionAmount = amount.trimmed().toDouble(&amountOk);
    if (!amountOk) {
        return failureResult(QStringLiteral("金额格式不正确"));
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
        return failureResult(validationError);
    }

    const int id = m_transactionRepository->addTransaction(transaction);
    if (id <= 0) {
        return failureResult(QStringLiteral("保存账单失败，请稍后重试"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), id}
    };
}

QVariantMap AppController::getTransactionById(int id) const
{
    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    const std::optional<Transaction> transaction = m_transactionRepository->getTransactionById(id);
    if (!transaction.has_value()) {
        return failureResult(QStringLiteral("未找到账单记录"));
    }

    return transactionToMap(transaction.value());
}

QVariantMap AppController::updateTransaction(int id,
                                             const QString &type,
                                             const QString &amount,
                                             const QString &category,
                                             const QString &date,
                                             const QString &note)
{
    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(type, &typeOk);
    if (!typeOk) {
        return failureResult(QStringLiteral("账单类型不正确"));
    }

    bool amountOk = false;
    const double transactionAmount = amount.trimmed().toDouble(&amountOk);
    if (!amountOk) {
        return failureResult(QStringLiteral("金额格式不正确"));
    }

    Transaction transaction(
        id,
        transactionType,
        transactionAmount,
        category.trimmed(),
        date.trimmed(),
        note.trimmed(),
        QString(),
        QString());

    QString validationError;
    if (!transaction.validate(&validationError)) {
        return failureResult(validationError);
    }

    if (!m_transactionRepository->updateTransaction(transaction)) {
        return failureResult(QStringLiteral("更新账单失败，请确认记录是否存在"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), id}
    };
}

QVariantMap AppController::deleteTransaction(int id)
{
    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    if (!m_transactionRepository->deleteTransaction(id)) {
        return failureResult(QStringLiteral("删除账单失败，请确认记录是否存在"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), id}
    };
}

QVariantMap AppController::currentMonthSummary() const
{
    if (!m_transactionRepository) {
        return QVariantMap {
            {QStringLiteral("success"), false},
            {QStringLiteral("errorMessage"), QStringLiteral("账单仓库未初始化")},
            {QStringLiteral("income"), 0.0},
            {QStringLiteral("expense"), 0.0},
            {QStringLiteral("balance"), 0.0}
        };
    }

    const QDate currentDate = QDate::currentDate();
    const QList<Transaction> transactions =
        m_transactionRepository->getTransactionsByMonth(currentDate.year(), currentDate.month());

    const SummaryService summaryService;
    const SummaryResult summary = summaryService.calculate(transactions);

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("income"), summary.totalIncome},
        {QStringLiteral("expense"), summary.totalExpense},
        {QStringLiteral("balance"), summary.balance}
    };
}
