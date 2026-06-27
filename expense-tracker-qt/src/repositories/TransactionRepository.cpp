#include "TransactionRepository.h"

#include "../models/Transaction.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <optional>

namespace {
std::optional<Transaction> transactionFromCurrentRow(const QSqlQuery &query)
{
    const int id = query.value(QStringLiteral("id")).toInt();

    bool typeOk = false;
    const TransactionType type = Transaction::typeFromString(query.value(QStringLiteral("type")).toString(), &typeOk);
    if (!typeOk) {
        qWarning().noquote() << "查询交易失败：发现非法交易类型，记录 id:" << id;
        return std::nullopt;
    }

    return Transaction(
        id,
        type,
        query.value(QStringLiteral("amount")).toDouble(),
        query.value(QStringLiteral("category")).toString(),
        query.value(QStringLiteral("date")).toString(),
        query.value(QStringLiteral("note")).toString(),
        query.value(QStringLiteral("created_at")).toString(),
        query.value(QStringLiteral("updated_at")).toString());
}
}

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

QList<Transaction> TransactionRepository::getAllTransactions()
{
    QList<Transaction> transactions;

    if (!m_database.isOpen()) {
        qWarning().noquote() << "查询交易失败：数据库未打开";
        return transactions;
    }

    QSqlQuery query(m_database);
    const bool prepared = query.prepare(QStringLiteral(R"(
        SELECT
            id,
            type,
            amount,
            category,
            date,
            note,
            created_at,
            updated_at
        FROM transactions
        ORDER BY date DESC
    )"));

    if (!prepared) {
        qWarning().noquote() << "查询交易失败：SQL 准备失败:" << query.lastError().text();
        return transactions;
    }

    if (!query.exec()) {
        qWarning().noquote() << "查询交易失败：SQL 执行失败:" << query.lastError().text();
        return transactions;
    }

    while (query.next()) {
        const std::optional<Transaction> transaction = transactionFromCurrentRow(query);
        if (transaction.has_value()) {
            transactions.append(transaction.value());
        }
    }

    return transactions;
}

std::optional<Transaction> TransactionRepository::getTransactionById(int id)
{
    if (!m_database.isOpen()) {
        qWarning().noquote() << "按 id 查询交易失败：数据库未打开";
        return std::nullopt;
    }

    QSqlQuery query(m_database);
    const bool prepared = query.prepare(QStringLiteral(R"(
        SELECT
            id,
            type,
            amount,
            category,
            date,
            note,
            created_at,
            updated_at
        FROM transactions
        WHERE id = :id
        LIMIT 1
    )"));

    if (!prepared) {
        qWarning().noquote() << "按 id 查询交易失败：SQL 准备失败:" << query.lastError().text();
        return std::nullopt;
    }

    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        qWarning().noquote() << "按 id 查询交易失败：SQL 执行失败:" << query.lastError().text();
        return std::nullopt;
    }

    if (!query.next()) {
        return std::nullopt;
    }

    return transactionFromCurrentRow(query);
}

bool TransactionRepository::updateTransaction(const Transaction &transaction)
{
    if (!m_database.isOpen()) {
        qWarning().noquote() << "更新交易失败：数据库未打开";
        return false;
    }

    QSqlQuery query(m_database);
    const bool prepared = query.prepare(QStringLiteral(R"(
        UPDATE transactions
        SET
            type = :type,
            amount = :amount,
            category = :category,
            date = :date,
            note = :note,
            updated_at = :updated_at
        WHERE id = :id
    )"));

    if (!prepared) {
        qWarning().noquote() << "更新交易失败：SQL 准备失败:" << query.lastError().text();
        return false;
    }

    query.bindValue(QStringLiteral(":id"), transaction.id());
    query.bindValue(QStringLiteral(":type"), Transaction::typeToString(transaction.type()));
    query.bindValue(QStringLiteral(":amount"), transaction.amount());
    query.bindValue(QStringLiteral(":category"), transaction.category());
    query.bindValue(QStringLiteral(":date"), transaction.date());
    query.bindValue(QStringLiteral(":note"), transaction.note());
    query.bindValue(QStringLiteral(":updated_at"), QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning().noquote() << "更新交易失败：SQL 执行失败:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}
