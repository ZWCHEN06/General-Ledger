#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("OpeningQtSmoke", "Main");

    bool ok = false;
    const int autoExitMs = qEnvironmentVariableIntValue("OPENING_QT_SMOKE_AUTO_EXIT_MS", &ok);
    if (ok && autoExitMs > 0) {
        QTimer::singleShot(autoExitMs, &app, &QCoreApplication::quit);
    }

    return app.exec();
}

