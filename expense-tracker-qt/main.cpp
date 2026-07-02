#include "src/AppController.h"
#include "src/database/DatabaseManager.h"
#include "src/models/CategoryListModel.h"
#include "src/models/TransactionListModel.h"
#include "src/repositories/CategoryRepository.h"
#include "src/repositories/TransactionRepository.h"

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSqlError>

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
        QString detail = databaseManager.lastErrorMessage();
        if (detail.trimmed().isEmpty()) {
            detail = databaseManager.database().lastError().text();
        }
        databaseErrorMessage = detail.isEmpty()
            ? QStringLiteral("数据库初始化失败，请检查 transactions 表结构和 SQLite 日志")
            : QStringLiteral("数据库初始化失败：%1").arg(detail);
        qWarning().noquote() << databaseErrorMessage;
    } else {
        databaseReady = true;
    }

    QQmlApplicationEngine engine;
    TransactionRepository transactionRepository(databaseManager.database());
    CategoryRepository categoryRepository(databaseManager.database());
    AppController appController(&transactionRepository);
    appController.setDatabaseStatus(databaseReady, databaseErrorMessage);

    TransactionListModel transactionListModel(&transactionRepository);
    CategoryListModel categoryListModel(&categoryRepository);
    appController.setTransactionListModel(&transactionListModel);
    appController.setCategoryRepository(&categoryRepository);
    appController.setCategoryListModel(&categoryListModel);
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &appController);
    engine.rootContext()->setContextProperty(QStringLiteral("transactionListModel"), &transactionListModel);
    engine.rootContext()->setContextProperty(QStringLiteral("categoryListModel"), &categoryListModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("ExpenseTracker", "Main");

    return app.exec();
}
