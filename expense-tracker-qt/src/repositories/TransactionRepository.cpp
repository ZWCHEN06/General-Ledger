#include "TransactionRepository.h"

#include "../models/Transaction.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

TransactionRepository::TransactionRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

int TransactionRepository::addTransaction(const Transaction &transaction)
{
    if (!m_database.isOpen()) {
        qWarning().noquote() << "添加交易失败：数据库未打开";
        return -1;
    }

    QSqlQuery query(m_database);
    const bool prepared = query.prepare(QStringLiteral(R"(
        INSERT INTO transactions (
            type,
            amount,
            category,
            date,
            note,
            created_at,
            updated_at
        ) VALUES (
            :type,
            :amount,
            :category,
            :date,
            :note,
            :created_at,
            :updated_at
        )
    )"));

    if (!prepared) {
        qWarning().noquote() << "添加交易失败：SQL 准备失败:" << query.lastError().text();
        return -1;
    }

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    query.bindValue(QStringLiteral(":type"), Transaction::typeToString(transaction.type()));
    query.bindValue(QStringLiteral(":amount"), transaction.amount());
    query.bindValue(QStringLiteral(":category"), transaction.category());
    query.bindValue(QStringLiteral(":date"), transaction.date());
    query.bindValue(QStringLiteral(":note"), transaction.note());
    query.bindValue(QStringLiteral(":created_at"), now);
    query.bindValue(QStringLiteral(":updated_at"), now);

    if (!query.exec()) {
        qWarning().noquote() << "添加交易失败：SQL 执行失败:" << query.lastError().text();
        return -1;
    }

    bool ok = false;
    const int id = query.lastInsertId().toInt(&ok);
    if (!ok) {
        qWarning().noquote() << "添加交易失败：无法获取新记录 id";
        return -1;
    }

    return id;
}
