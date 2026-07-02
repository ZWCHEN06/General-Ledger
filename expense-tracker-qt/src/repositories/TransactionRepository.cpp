#include "TransactionRepository.h"

#include "../models/TransactionFilter.h"
#include "../models/Transaction.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

#include <algorithm>
#include <optional>

namespace {
std::optional<Transaction> transactionFromCurrentRow(const QSqlQuery &query)
{
    const int id = query.value(QStringLiteral("id")).toInt();
    const QVariant categoryIdValue = query.value(QStringLiteral("category_id"));
    const std::optional<int> categoryId = categoryIdValue.isNull()
        ? std::nullopt
        : std::optional<int>(categoryIdValue.toInt());

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
        query.value(QStringLiteral("updated_at")).toString(),
        categoryId);
}

QDate parseStoredTransactionDate(const QString &date)
{
    const QString trimmedDate = date.trimmed();
    QString separatorNormalizedDate = trimmedDate;
    separatorNormalizedDate.replace(QStringLiteral("/"), QStringLiteral("-"));
    separatorNormalizedDate.replace(QStringLiteral("."), QStringLiteral("-"));

    const QStringList candidates {
        trimmedDate,
        separatorNormalizedDate
    };
    const QStringList formats {
        QStringLiteral("yyyy-MM-dd"),
        QStringLiteral("yyyy-M-d"),
        QStringLiteral("yyyyMMdd")
    };

    for (const QString &candidate : candidates) {
        for (const QString &format : formats) {
            const QDate parsedDate = QDate::fromString(candidate, format);
            if (parsedDate.isValid()) {
                return parsedDate;
            }
        }
    }

    return QDate();
}

bool transactionMatchesDateFilter(const Transaction &transaction, const TransactionFilter &filter)
{
    if (!filter.year.has_value() && !filter.month.has_value()) {
        return true;
    }

    const QDate transactionDate = parseStoredTransactionDate(transaction.date());
    if (!transactionDate.isValid()) {
        return false;
    }

    if (filter.year.has_value() && transactionDate.year() != filter.year.value()) {
        return false;
    }

    if (filter.month.has_value() && transactionDate.month() != filter.month.value()) {
        return false;
    }

    return true;
}

QString escapeSqlLikePattern(QString keyword)
{
    keyword.replace(QStringLiteral("!"), QStringLiteral("!!"));
    keyword.replace(QStringLiteral("%"), QStringLiteral("!%"));
    keyword.replace(QStringLiteral("_"), QStringLiteral("!_"));
    return keyword;
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
            category_id,
            date,
            note,
            created_at,
            updated_at
        ) VALUES (
            :type,
            :amount,
            :category,
            :category_id,
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
    query.bindValue(QStringLiteral(":category_id"),
                    transaction.categoryId().has_value()
                        ? QVariant(transaction.categoryId().value())
                        : QVariant());
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
            transactions.id AS id,
            transactions.type AS type,
            transactions.amount AS amount,
            COALESCE(categories.name, transactions.category) AS category,
            transactions.category_id AS category_id,
            transactions.date AS date,
            transactions.note AS note,
            transactions.created_at AS created_at,
            transactions.updated_at AS updated_at
        FROM transactions
        LEFT JOIN categories
          ON categories.id = transactions.category_id
         AND categories.type = transactions.type
        ORDER BY transactions.date DESC, transactions.created_at DESC
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

QList<Transaction> TransactionRepository::getTransactionsByFilter(const TransactionFilter &filter)
{
    QList<Transaction> transactions;

    if (!m_database.isOpen()) {
        qWarning().noquote() << "筛选查询交易失败：数据库未打开";
        return transactions;
    }

    QString validationError;
    if (!filter.validate(&validationError)) {
        qWarning().noquote() << "筛选查询交易失败：" << validationError;
        return transactions;
    }

    QStringList whereConditions;

    if (filter.type.has_value()) {
        whereConditions.append(QStringLiteral("transactions.type = :type"));
    }

    const QString category = filter.category.value_or(QString()).trimmed();
    if (!category.isEmpty()) {
        whereConditions.append(QStringLiteral("transactions.category = :category"));
    }

    const QString keyword = filter.keyword.value_or(QString()).trimmed();
    if (!keyword.isEmpty()) {
        whereConditions.append(QStringLiteral("transactions.note LIKE :keyword ESCAPE '!'"));
    }

    if (filter.minAmount.has_value()) {
        whereConditions.append(QStringLiteral("transactions.amount >= :minAmount"));
    }

    if (filter.maxAmount.has_value()) {
        whereConditions.append(QStringLiteral("transactions.amount <= :maxAmount"));
    }

    QString sql = QStringLiteral(R"(
        SELECT
            transactions.id AS id,
            transactions.type AS type,
            transactions.amount AS amount,
            COALESCE(categories.name, transactions.category) AS category,
            transactions.category_id AS category_id,
            transactions.date AS date,
            transactions.note AS note,
            transactions.created_at AS created_at,
            transactions.updated_at AS updated_at
        FROM transactions
        LEFT JOIN categories
          ON categories.id = transactions.category_id
         AND categories.type = transactions.type
    )");

    if (!whereConditions.isEmpty()) {
        sql.append(QStringLiteral("        WHERE "));
        sql.append(whereConditions.join(QStringLiteral("\n          AND ")));
        sql.append(QLatin1Char('\n'));
    }

    sql.append(QStringLiteral("        ORDER BY transactions.date DESC, transactions.created_at DESC"));

    QSqlQuery query(m_database);
    const bool prepared = query.prepare(sql);

    if (!prepared) {
        qWarning().noquote() << "筛选查询交易失败：SQL 准备失败:" << query.lastError().text();
        return transactions;
    }

    if (filter.type.has_value()) {
        query.bindValue(QStringLiteral(":type"), Transaction::typeToString(filter.type.value()));
    }

    if (!category.isEmpty()) {
        query.bindValue(QStringLiteral(":category"), category);
    }

    if (!keyword.isEmpty()) {
        query.bindValue(QStringLiteral(":keyword"),
                        QStringLiteral("%") + escapeSqlLikePattern(keyword) + QStringLiteral("%"));
    }

    if (filter.minAmount.has_value()) {
        query.bindValue(QStringLiteral(":minAmount"), filter.minAmount.value());
    }

    if (filter.maxAmount.has_value()) {
        query.bindValue(QStringLiteral(":maxAmount"), filter.maxAmount.value());
    }

    if (!query.exec()) {
        qWarning().noquote() << "筛选查询交易失败：SQL 执行失败:" << query.lastError().text();
        return transactions;
    }

    while (query.next()) {
        const std::optional<Transaction> transaction = transactionFromCurrentRow(query);
        if (transaction.has_value() && transactionMatchesDateFilter(transaction.value(), filter)) {
            transactions.append(transaction.value());
        }
    }

    return transactions;
}

QList<Transaction> TransactionRepository::getTransactionsByMonth(int year, int month)
{
    QList<Transaction> transactions;

    if (!m_database.isOpen()) {
        qWarning().noquote() << "按月份查询交易失败：数据库未打开";
        return transactions;
    }

    const QDate startDate(year, month, 1);
    if (!startDate.isValid()) {
        qWarning().noquote() << "按月份查询交易失败：年份或月份非法，year:" << year << "month:" << month;
        return transactions;
    }

    const QDate endDate = startDate.addMonths(1);

    QSqlQuery query(m_database);
    const bool prepared = query.prepare(QStringLiteral(R"(
        SELECT
            transactions.id AS id,
            transactions.type AS type,
            transactions.amount AS amount,
            COALESCE(categories.name, transactions.category) AS category,
            transactions.category_id AS category_id,
            transactions.date AS date,
            transactions.note AS note,
            transactions.created_at AS created_at,
            transactions.updated_at AS updated_at
        FROM transactions
        LEFT JOIN categories
          ON categories.id = transactions.category_id
         AND categories.type = transactions.type
        ORDER BY transactions.date DESC, transactions.id DESC
    )"));

    if (!prepared) {
        qWarning().noquote() << "按月份查询交易失败：SQL 准备失败:" << query.lastError().text();
        return transactions;
    }

    if (!query.exec()) {
        qWarning().noquote() << "按月份查询交易失败：SQL 执行失败:" << query.lastError().text();
        return transactions;
    }

    while (query.next()) {
        const std::optional<Transaction> transaction = transactionFromCurrentRow(query);
        if (transaction.has_value()) {
            const QDate transactionDate = parseStoredTransactionDate(transaction->date());
            if (transactionDate >= startDate && transactionDate < endDate) {
                transactions.append(transaction.value());
            }
        }
    }

    std::sort(transactions.begin(), transactions.end(), [](const Transaction &left, const Transaction &right) {
        const QDate leftDate = parseStoredTransactionDate(left.date());
        const QDate rightDate = parseStoredTransactionDate(right.date());
        if (leftDate == rightDate) {
            return left.id() > right.id();
        }
        return leftDate > rightDate;
    });

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
            transactions.id AS id,
            transactions.type AS type,
            transactions.amount AS amount,
            COALESCE(categories.name, transactions.category) AS category,
            transactions.category_id AS category_id,
            transactions.date AS date,
            transactions.note AS note,
            transactions.created_at AS created_at,
            transactions.updated_at AS updated_at
        FROM transactions
        LEFT JOIN categories
          ON categories.id = transactions.category_id
         AND categories.type = transactions.type
        WHERE transactions.id = :id
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
            category_id = :category_id,
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
    query.bindValue(QStringLiteral(":category_id"),
                    transaction.categoryId().has_value()
                        ? QVariant(transaction.categoryId().value())
                        : QVariant());
    query.bindValue(QStringLiteral(":date"), transaction.date());
    query.bindValue(QStringLiteral(":note"), transaction.note());
    query.bindValue(QStringLiteral(":updated_at"), QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning().noquote() << "更新交易失败：SQL 执行失败:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool TransactionRepository::deleteTransaction(int id)
{
    if (!m_database.isOpen()) {
        qWarning().noquote() << "删除交易失败：数据库未打开";
        return false;
    }

    QSqlQuery query(m_database);
    const bool prepared = query.prepare(QStringLiteral(R"(
        DELETE FROM transactions
        WHERE id = :id
    )"));

    if (!prepared) {
        qWarning().noquote() << "删除交易失败：SQL 准备失败:" << query.lastError().text();
        return false;
    }

    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        qWarning().noquote() << "删除交易失败：SQL 执行失败:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}
