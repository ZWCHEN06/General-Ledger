#include "CategoryRepository.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

CategoryRepository::CategoryRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

CategoryRepositoryResult CategoryRepository::addCategory(const QString &name, TransactionType type)
{
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return CategoryRepositoryResult {
            false,
            0,
            QStringLiteral("分类名称不能为空")
        };
    }

    switch (type) {
    case TransactionType::Income:
    case TransactionType::Expense:
        break;
    default:
        return CategoryRepositoryResult {
            false,
            0,
            QStringLiteral("分类类型必须是收入或支出")
        };
    }

    if (!m_database.isOpen()) {
        return CategoryRepositoryResult {
            false,
            0,
            QStringLiteral("数据库未打开，无法新增分类")
        };
    }

    const QString typeText = Transaction::typeToString(type);

    QSqlQuery duplicateQuery(m_database);
    if (!duplicateQuery.prepare(QStringLiteral(R"(
        SELECT id
        FROM categories
        WHERE name = :name
          AND type = :type
        LIMIT 1
    )"))) {
        const QString error = QStringLiteral("检查分类重名失败：%1").arg(duplicateQuery.lastError().text());
        qWarning().noquote() << error;
        return CategoryRepositoryResult {false, 0, error};
    }

    duplicateQuery.bindValue(QStringLiteral(":name"), trimmedName);
    duplicateQuery.bindValue(QStringLiteral(":type"), typeText);

    if (!duplicateQuery.exec()) {
        const QString error = QStringLiteral("检查分类重名失败：%1").arg(duplicateQuery.lastError().text());
        qWarning().noquote() << error;
        return CategoryRepositoryResult {false, 0, error};
    }

    if (duplicateQuery.next()) {
        return CategoryRepositoryResult {
            false,
            0,
            QStringLiteral("同一类型下分类名称不能重复")
        };
    }

    QSqlQuery sortOrderQuery(m_database);
    if (!sortOrderQuery.prepare(QStringLiteral(R"(
        SELECT COALESCE(MAX(sort_order), -1)
        FROM categories
        WHERE type = :type
    )"))) {
        const QString error = QStringLiteral("获取分类排序失败：%1").arg(sortOrderQuery.lastError().text());
        qWarning().noquote() << error;
        return CategoryRepositoryResult {false, 0, error};
    }

    sortOrderQuery.bindValue(QStringLiteral(":type"), typeText);

    if (!sortOrderQuery.exec() || !sortOrderQuery.next()) {
        const QString error = QStringLiteral("获取分类排序失败：%1").arg(sortOrderQuery.lastError().text());
        qWarning().noquote() << error;
        return CategoryRepositoryResult {false, 0, error};
    }

    const int sortOrder = sortOrderQuery.value(0).toInt() + 1;
    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlQuery insertQuery(m_database);
    if (!insertQuery.prepare(QStringLiteral(R"(
        INSERT INTO categories (
            name,
            type,
            is_default,
            sort_order,
            created_at,
            updated_at
        ) VALUES (
            :name,
            :type,
            0,
            :sort_order,
            :created_at,
            :updated_at
        )
    )"))) {
        const QString error = QStringLiteral("新增分类失败：%1").arg(insertQuery.lastError().text());
        qWarning().noquote() << error;
        return CategoryRepositoryResult {false, 0, error};
    }

    insertQuery.bindValue(QStringLiteral(":name"), trimmedName);
    insertQuery.bindValue(QStringLiteral(":type"), typeText);
    insertQuery.bindValue(QStringLiteral(":sort_order"), sortOrder);
    insertQuery.bindValue(QStringLiteral(":created_at"), now);
    insertQuery.bindValue(QStringLiteral(":updated_at"), now);

    if (!insertQuery.exec()) {
        const QString error = QStringLiteral("新增分类失败：%1").arg(insertQuery.lastError().text());
        qWarning().noquote() << error;
        return CategoryRepositoryResult {false, 0, error};
    }

    const int id = insertQuery.lastInsertId().toInt();
    if (id <= 0) {
        const QString error = QStringLiteral("新增分类失败：未能获取新分类 id");
        qWarning().noquote() << error;
        return CategoryRepositoryResult {false, 0, error};
    }

    return CategoryRepositoryResult {
        true,
        id,
        QString()
    };
}
