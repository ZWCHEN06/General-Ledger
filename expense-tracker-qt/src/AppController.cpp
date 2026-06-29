#include "AppController.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

QString AppController::testMessage() const
{
    return QStringLiteral("C++ 已连接");
}

