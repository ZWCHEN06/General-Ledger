#include "src/database/DatabaseManager.h"

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

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
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("ExpenseTracker", "Main");

    return app.exec();
}
