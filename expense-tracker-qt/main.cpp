#include "src/AppController.h"
#include "src/database/DatabaseManager.h"
#include "src/models/TransactionListModel.h"
#include "src/repositories/TransactionRepository.h"

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    DatabaseManager databaseManager;
    if (!databaseManager.openDatabase()) {
        qWarning().noquote() << "打开 SQLite 数据库失败，界面将继续启动。";
    } else if (!databaseManager.initializeTables()) {
        qWarning().noquote() << "初始化数据库表失败，界面将继续启动。";
    }

    QQmlApplicationEngine engine;
    TransactionRepository transactionRepository(databaseManager.database());
    AppController appController(&transactionRepository);
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
