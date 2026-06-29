#pragma once

#include <QObject>
#include <QString>

class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);

    Q_INVOKABLE QString testMessage() const;
};

