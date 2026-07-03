#include "SubcategoryRepository.h"

#include <QDebug>
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
