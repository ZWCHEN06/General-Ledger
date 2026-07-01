#include "src/AppController.h"
#include "src/database/DatabaseManager.h"
#include "src/models/TransactionFilter.h"
#include "src/models/TransactionListModel.h"
#include "src/repositories/TransactionRepository.h"

#include <QDate>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSqlError>

namespace {
#ifdef QT_DEBUG
void printFilterResultCount(const QString &label,
                            TransactionRepository &transactionRepository,
                            const TransactionFilter &filter)
{
    const QList<Transaction> transactions = transactionRepository.getTransactionsByFilter(filter);
    qInfo().noquote() << QStringLiteral("[手工验证][getTransactionsByFilter] %1: %2 条")
                             .arg(label)
                             .arg(transactions.size());
}

void runTransactionFilterManualChecks(TransactionRepository &transactionRepository)
{
    qInfo().noquote() << QStringLiteral("[手工验证][getTransactionsByFilter] 开始");

    TransactionFilter emptyFilter;
    printFilterResultCount(QStringLiteral("无筛选条件"), transactionRepository, emptyFilter);

    const QDate currentDate = QDate::currentDate();
    TransactionFilter monthFilter;
    monthFilter.year = currentDate.year();
    monthFilter.month = currentDate.month();
    printFilterResultCount(QStringLiteral("当前月份"), transactionRepository, monthFilter);

    TransactionFilter expenseFilter;
    expenseFilter.type = TransactionType::Expense;
    printFilterResultCount(QStringLiteral("支出类型"), transactionRepository, expenseFilter);

    TransactionFilter categoryFilter;
    categoryFilter.category = QStringLiteral("餐饮");
    printFilterResultCount(QStringLiteral("分类=餐饮"), transactionRepository, categoryFilter);

    TransactionFilter keywordFilter;
    keywordFilter.keyword = QStringLiteral("午餐");
    printFilterResultCount(QStringLiteral("备注包含午餐"), transactionRepository, keywordFilter);

    TransactionFilter amountFilter;
    amountFilter.minAmount = 10.0;
    amountFilter.maxAmount = 100.0;
    printFilterResultCount(QStringLiteral("金额 10 到 100"), transactionRepository, amountFilter);

    TransactionFilter combinedFilter;
    combinedFilter.year = currentDate.year();
    combinedFilter.month = currentDate.month();
    combinedFilter.type = TransactionType::Expense;
    combinedFilter.minAmount = 10.0;
    combinedFilter.maxAmount = 100.0;
    printFilterResultCount(QStringLiteral("当前月份支出且金额 10 到 100"), transactionRepository, combinedFilter);

    qInfo().noquote() << QStringLiteral("[手工验证][getTransactionsByFilter] 结束");
}
#endif
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("ExpenseTracker"));
    QCoreApplication::setApplicationName(QStringLiteral("expense-tracker-qt"));

    DatabaseManager databaseManager;
    bool databaseReady = false;
    QString databaseErrorMessage;

    if (!databaseManager.openDatabase()) {
        const QString detail = databaseManager.database().lastError().text();
        databaseErrorMessage = detail.isEmpty()
            ? QStringLiteral("数据库打开失败，请检查数据库路径和文件权限")
            : QStringLiteral("数据库打开失败：%1").arg(detail);
        qWarning().noquote() << databaseErrorMessage;
    } else if (!databaseManager.initializeTables()) {
        const QString detail = databaseManager.database().lastError().text();
        databaseErrorMessage = detail.isEmpty()
            ? QStringLiteral("数据库初始化失败，请检查 transactions 表结构和 SQLite 日志")
            : QStringLiteral("数据库初始化失败：%1").arg(detail);
        qWarning().noquote() << databaseErrorMessage;
    } else {
        databaseReady = true;
    }

    QQmlApplicationEngine engine;
    TransactionRepository transactionRepository(databaseManager.database());
    AppController appController(&transactionRepository);
    appController.setDatabaseStatus(databaseReady, databaseErrorMessage);

#ifdef QT_DEBUG
    if (databaseReady) {
        runTransactionFilterManualChecks(transactionRepository);
    }
#endif

    TransactionListModel transactionListModel(&transactionRepository);
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &appController);
    engine.rootContext()->setContextProperty(QStringLiteral("transactionListModel"), &transactionListModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("ExpenseTracker", "Main");

    return app.exec();
}
