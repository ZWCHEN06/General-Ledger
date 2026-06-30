#include "AppController.h"

#include "models/Transaction.h"
#include "repositories/TransactionRepository.h"
#include "services/CsvExportService.h"
#include "services/SummaryService.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

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

bool AppController::databaseReady() const
{
    return m_databaseReady;
}

QString AppController::databaseErrorMessage() const
{
    return effectiveDatabaseErrorMessage();
}

void AppController::setDatabaseStatus(bool ready, const QString &errorMessage)
{
    if (m_databaseReady == ready && m_databaseErrorMessage == errorMessage) {
        return;
    }

    m_databaseReady = ready;
    m_databaseErrorMessage = errorMessage;
    emit databaseStatusChanged();
}

QString AppController::effectiveDatabaseErrorMessage() const
{
    if (!m_databaseErrorMessage.trimmed().isEmpty()) {
        return m_databaseErrorMessage;
    }

    return QStringLiteral("数据库不可用，请检查 SQLite 打开和初始化日志");
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
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

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
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

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
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

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
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

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
    if (!m_databaseReady) {
        return QVariantMap {
            {QStringLiteral("success"), false},
            {QStringLiteral("errorMessage"), effectiveDatabaseErrorMessage()},
            {QStringLiteral("income"), 0.0},
            {QStringLiteral("expense"), 0.0},
            {QStringLiteral("balance"), 0.0}
        };
    }

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

QVariantMap AppController::exportCsv() const
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    const QList<Transaction> transactions = m_transactionRepository->getAllTransactions();
    if (transactions.isEmpty()) {
        return failureResult(QStringLiteral("暂无可导出的账单"));
    }

    const CsvExportService csvExportService;
    const QString csvContent = csvExportService.exportTransactions(transactions);

#ifdef Q_OS_ANDROID
    Q_UNUSED(csvContent)
    return failureResult(QStringLiteral("当前 Android 版暂未接入系统文件保存或分享能力，无法导出 CSV"));
#else
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documentsPath.trimmed().isEmpty()) {
        documentsPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    if (documentsPath.trimmed().isEmpty()) {
        return failureResult(QStringLiteral("无法获取可写入的导出目录"));
    }

    QDir documentsDir(documentsPath);
    if (!documentsDir.exists() && !QDir().mkpath(documentsPath)) {
        return failureResult(QStringLiteral("无法创建导出目录：%1").arg(documentsPath));
    }

    const QString fileName = QStringLiteral("expense_transactions_%1.csv")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString filePath = documentsDir.filePath(fileName);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return failureResult(QStringLiteral("无法创建 CSV 文件：%1").arg(file.errorString()));
    }

    QByteArray output;
    output.append("\xEF\xBB\xBF", 3);
    output.append(csvContent.toUtf8());

    const qint64 bytesWritten = file.write(output);
    file.close();

    if (bytesWritten != output.size()) {
        QFile::remove(filePath);
        return failureResult(QStringLiteral("写入 CSV 文件失败"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("filePath"), filePath}
    };
#endif
}
