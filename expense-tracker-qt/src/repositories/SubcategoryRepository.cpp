#include "SubcategoryRepository.h"

#include <QDebug>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>

namespace {
Subcategory subcategoryFromCurrentRow(const QSqlQuery &query)
{
    return Subcategory(
        query.value(QStringLiteral("id")).toInt(),
        query.value(QStringLiteral("category_id")).toInt(),
        query.value(QStringLiteral("name")).toString(),
        query.value(QStringLiteral("is_default")).toInt() == 1,
        query.value(QStringLiteral("sort_order")).toInt(),
        query.value(QStringLiteral("created_at")).toString(),
        query.value(QStringLiteral("updated_at")).toString());
}
}

SubcategoryRepository::SubcategoryRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

QList<Subcategory> SubcategoryRepository::getSubcategoriesByCategoryId(int categoryId)
{
    QList<Subcategory> subcategories;

    if (categoryId <= 0) {
        qWarning().noquote() << "查询二级分类失败：一级分类 id 必须大于 0";
        return subcategories;
    }

    if (!m_database.isOpen()) {
        qWarning().noquote() << "查询二级分类失败：数据库未打开";
        return subcategories;
    }

    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        SELECT
            id,
            category_id,
            name,
            is_default,
            sort_order,
            created_at,
            updated_at
        FROM subcategories
        WHERE category_id = :category_id
        ORDER BY sort_order ASC, id ASC
    )"))) {
        qWarning().noquote() << "查询二级分类失败：SQL 准备失败:" << query.lastError().text();
        return subcategories;
    }

    query.bindValue(QStringLiteral(":category_id"), categoryId);

    if (!query.exec()) {
        qWarning().noquote() << "查询二级分类失败：SQL 执行失败:" << query.lastError().text();
        return subcategories;
    }

    while (query.next()) {
        subcategories.append(subcategoryFromCurrentRow(query));
    }

    return subcategories;
}

QList<Subcategory> SubcategoryRepository::getSubcategoryById(int id)
{
    QList<Subcategory> subcategories;

    if (id <= 0) {
        qWarning().noquote() << "按 id 查询二级分类失败：二级分类 id 必须大于 0";
        return subcategories;
    }

    if (!m_database.isOpen()) {
        qWarning().noquote() << "按 id 查询二级分类失败：数据库未打开";
        return subcategories;
    }

    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        SELECT
            id,
            category_id,
            name,
            is_default,
            sort_order,
            created_at,
            updated_at
        FROM subcategories
        WHERE id = :id
        ORDER BY sort_order ASC, id ASC
        LIMIT 1
    )"))) {
        qWarning().noquote() << "按 id 查询二级分类失败：SQL 准备失败:" << query.lastError().text();
        return subcategories;
    }

    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        qWarning().noquote() << "按 id 查询二级分类失败：SQL 执行失败:" << query.lastError().text();
        return subcategories;
    }

    while (query.next()) {
        subcategories.append(subcategoryFromCurrentRow(query));
    }

    return subcategories;
}

QList<Subcategory> SubcategoryRepository::findSubcategoryByNameAndCategoryId(const QString &name, int categoryId)
{
    QList<Subcategory> subcategories;

    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        qWarning().noquote() << "按名称查询二级分类失败：二级分类名称不能为空";
        return subcategories;
    }

    if (categoryId <= 0) {
        qWarning().noquote() << "按名称查询二级分类失败：一级分类 id 必须大于 0";
        return subcategories;
    }

    if (!m_database.isOpen()) {
        qWarning().noquote() << "按名称查询二级分类失败：数据库未打开";
        return subcategories;
    }

    QSqlQuery query(m_database);
    if (!query.prepare(QStringLiteral(R"(
        SELECT
            id,
            category_id,
            name,
            is_default,
            sort_order,
            created_at,
            updated_at
        FROM subcategories
        WHERE category_id = :category_id
          AND name = :name
        ORDER BY sort_order ASC, id ASC
        LIMIT 1
    )"))) {
        qWarning().noquote() << "按名称查询二级分类失败：SQL 准备失败:" << query.lastError().text();
        return subcategories;
    }

    query.bindValue(QStringLiteral(":category_id"), categoryId);
    query.bindValue(QStringLiteral(":name"), trimmedName);

    if (!query.exec()) {
        qWarning().noquote() << "按名称查询二级分类失败：SQL 执行失败:" << query.lastError().text();
        return subcategories;
    }

    while (query.next()) {
        subcategories.append(subcategoryFromCurrentRow(query));
    }

    return subcategories;
}

SubcategoryRepositoryResult SubcategoryRepository::addSubcategory(int categoryId, const QString &name)
{
    if (categoryId <= 0) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("一级分类 id 必须大于 0")
        };
    }

    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类名称不能为空")
        };
    }

    if (!m_database.isOpen()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("数据库未打开，无法新增二级分类")
        };
    }

    QSqlQuery categoryQuery(m_database);
    if (!categoryQuery.prepare(QStringLiteral(R"(
        SELECT id
        FROM categories
        WHERE id = :category_id
        LIMIT 1
    )"))) {
        const QString error = QStringLiteral("检查一级分类是否存在失败：%1").arg(categoryQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    categoryQuery.bindValue(QStringLiteral(":category_id"), categoryId);

    if (!categoryQuery.exec()) {
        const QString error = QStringLiteral("检查一级分类是否存在失败：%1").arg(categoryQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (!categoryQuery.next()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("一级分类不存在")
        };
    }

    QSqlQuery duplicateQuery(m_database);
    if (!duplicateQuery.prepare(QStringLiteral(R"(
        SELECT id
        FROM subcategories
        WHERE category_id = :category_id
          AND name = :name
        LIMIT 1
    )"))) {
        const QString error = QStringLiteral("检查二级分类重名失败：%1").arg(duplicateQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    duplicateQuery.bindValue(QStringLiteral(":category_id"), categoryId);
    duplicateQuery.bindValue(QStringLiteral(":name"), trimmedName);

    if (!duplicateQuery.exec()) {
        const QString error = QStringLiteral("检查二级分类重名失败：%1").arg(duplicateQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (duplicateQuery.next()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("同一一级分类下二级分类名称不能重复")
        };
    }

    QSqlQuery sortOrderQuery(m_database);
    if (!sortOrderQuery.prepare(QStringLiteral(R"(
        SELECT COALESCE(MAX(sort_order), -1)
        FROM subcategories
        WHERE category_id = :category_id
    )"))) {
        const QString error = QStringLiteral("获取二级分类排序失败：%1").arg(sortOrderQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    sortOrderQuery.bindValue(QStringLiteral(":category_id"), categoryId);

    if (!sortOrderQuery.exec() || !sortOrderQuery.next()) {
        const QString error = QStringLiteral("获取二级分类排序失败：%1").arg(sortOrderQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    const int sortOrder = sortOrderQuery.value(0).toInt() + 1;
    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlQuery insertQuery(m_database);
    if (!insertQuery.prepare(QStringLiteral(R"(
        INSERT INTO subcategories (
            category_id,
            name,
            is_default,
            sort_order,
            created_at,
            updated_at
        ) VALUES (
            :category_id,
            :name,
            0,
            :sort_order,
            :created_at,
            :updated_at
        )
    )"))) {
        const QString error = QStringLiteral("新增二级分类失败：%1").arg(insertQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    insertQuery.bindValue(QStringLiteral(":category_id"), categoryId);
    insertQuery.bindValue(QStringLiteral(":name"), trimmedName);
    insertQuery.bindValue(QStringLiteral(":sort_order"), sortOrder);
    insertQuery.bindValue(QStringLiteral(":created_at"), now);
    insertQuery.bindValue(QStringLiteral(":updated_at"), now);

    if (!insertQuery.exec()) {
        const QString error = QStringLiteral("新增二级分类失败：%1").arg(insertQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    const int id = insertQuery.lastInsertId().toInt();
    if (id <= 0) {
        const QString error = QStringLiteral("新增二级分类失败：未能获取新二级分类 id");
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    return SubcategoryRepositoryResult {
        true,
        id,
        QString()
    };
}

SubcategoryRepositoryResult SubcategoryRepository::updateSubcategoryName(int id, const QString &name)
{
    if (id <= 0) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类 id 必须大于 0")
        };
    }

    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类名称不能为空")
        };
    }

    if (!m_database.isOpen()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("数据库未打开，无法修改二级分类")
        };
    }

    QSqlQuery subcategoryQuery(m_database);
    if (!subcategoryQuery.prepare(QStringLiteral(R"(
        SELECT
            category_id,
            is_default
        FROM subcategories
        WHERE id = :id
        LIMIT 1
    )"))) {
        const QString error = QStringLiteral("查询二级分类失败：%1").arg(subcategoryQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    subcategoryQuery.bindValue(QStringLiteral(":id"), id);

    if (!subcategoryQuery.exec()) {
        const QString error = QStringLiteral("查询二级分类失败：%1").arg(subcategoryQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (!subcategoryQuery.next()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类不存在")
        };
    }

    const int categoryId = subcategoryQuery.value(QStringLiteral("category_id")).toInt();
    const bool isDefault = subcategoryQuery.value(QStringLiteral("is_default")).toInt() == 1;
    if (isDefault) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("默认二级分类不能改名")
        };
    }

    QSqlQuery duplicateQuery(m_database);
    if (!duplicateQuery.prepare(QStringLiteral(R"(
        SELECT id
        FROM subcategories
        WHERE category_id = :category_id
          AND name = :name
          AND id <> :id
        LIMIT 1
    )"))) {
        const QString error = QStringLiteral("检查二级分类重名失败：%1").arg(duplicateQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    duplicateQuery.bindValue(QStringLiteral(":category_id"), categoryId);
    duplicateQuery.bindValue(QStringLiteral(":name"), trimmedName);
    duplicateQuery.bindValue(QStringLiteral(":id"), id);

    if (!duplicateQuery.exec()) {
        const QString error = QStringLiteral("检查二级分类重名失败：%1").arg(duplicateQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (duplicateQuery.next()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("同一一级分类下二级分类名称不能重复")
        };
    }

    QSqlQuery updateQuery(m_database);
    if (!updateQuery.prepare(QStringLiteral(R"(
        UPDATE subcategories
        SET
            name = :name,
            updated_at = :updated_at
        WHERE id = :id
    )"))) {
        const QString error = QStringLiteral("修改二级分类名称失败：%1").arg(updateQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    updateQuery.bindValue(QStringLiteral(":name"), trimmedName);
    updateQuery.bindValue(QStringLiteral(":updated_at"), QDateTime::currentDateTime().toString(Qt::ISODate));
    updateQuery.bindValue(QStringLiteral(":id"), id);

    if (!updateQuery.exec()) {
        const QString error = QStringLiteral("修改二级分类名称失败：%1").arg(updateQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (updateQuery.numRowsAffected() <= 0) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类不存在")
        };
    }

    return SubcategoryRepositoryResult {
        true,
        id,
        QString()
    };
}

SubcategoryRepositoryResult SubcategoryRepository::deleteSubcategory(int id)
{
    if (id <= 0) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类 id 必须大于 0")
        };
    }

    if (!m_database.isOpen()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("数据库未打开，无法删除二级分类")
        };
    }

    QSqlQuery subcategoryQuery(m_database);
    if (!subcategoryQuery.prepare(QStringLiteral(R"(
        SELECT
            category_id,
            name,
            is_default
        FROM subcategories
        WHERE id = :id
        LIMIT 1
    )"))) {
        const QString error = QStringLiteral("查询二级分类失败：%1").arg(subcategoryQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    subcategoryQuery.bindValue(QStringLiteral(":id"), id);

    if (!subcategoryQuery.exec()) {
        const QString error = QStringLiteral("查询二级分类失败：%1").arg(subcategoryQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (!subcategoryQuery.next()) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类不存在")
        };
    }

    const int categoryId = subcategoryQuery.value(QStringLiteral("category_id")).toInt();
    const QString name = subcategoryQuery.value(QStringLiteral("name")).toString();
    const bool isDefault = subcategoryQuery.value(QStringLiteral("is_default")).toInt() == 1;
    if (isDefault) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("默认二级分类不能删除")
        };
    }

    QSqlQuery usageQuery(m_database);
    if (!usageQuery.prepare(QStringLiteral(R"(
        SELECT COUNT(*)
        FROM transactions
        WHERE subcategory_id = :id
           OR (
               subcategory_id IS NULL
               AND subcategory = :name
               AND category_id = :category_id
           )
    )"))) {
        const QString error = QStringLiteral("检查二级分类使用状态失败：%1").arg(usageQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    usageQuery.bindValue(QStringLiteral(":id"), id);
    usageQuery.bindValue(QStringLiteral(":name"), name);
    usageQuery.bindValue(QStringLiteral(":category_id"), categoryId);

    if (!usageQuery.exec() || !usageQuery.next()) {
        const QString error = QStringLiteral("检查二级分类使用状态失败：%1").arg(usageQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (usageQuery.value(0).toInt() > 0) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("该二级分类已被账单使用，不能删除")
        };
    }

    QSqlQuery deleteQuery(m_database);
    if (!deleteQuery.prepare(QStringLiteral(R"(
        DELETE FROM subcategories
        WHERE id = :id
    )"))) {
        const QString error = QStringLiteral("删除二级分类失败：%1").arg(deleteQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    deleteQuery.bindValue(QStringLiteral(":id"), id);

    if (!deleteQuery.exec()) {
        const QString error = QStringLiteral("删除二级分类失败：%1").arg(deleteQuery.lastError().text());
        qWarning().noquote() << error;
        return SubcategoryRepositoryResult {false, 0, error};
    }

    if (deleteQuery.numRowsAffected() <= 0) {
        return SubcategoryRepositoryResult {
            false,
            0,
            QStringLiteral("二级分类不存在")
        };
    }

    return SubcategoryRepositoryResult {
        true,
        id,
        QString()
    };
}
