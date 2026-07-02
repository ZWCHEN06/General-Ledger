#include "CategoryListModel.h"

#include "../repositories/CategoryRepository.h"

CategoryListModel::CategoryListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

CategoryListModel::CategoryListModel(CategoryRepository *categoryRepository, QObject *parent)
    : QAbstractListModel(parent),
      m_categoryRepository(categoryRepository)
{
}

int CategoryListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_categories.size();
}

QVariant CategoryListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_categories.size()) {
        return {};
    }

    const Category &category = m_categories.at(index.row());

    switch (role) {
    case IdRole:
    case CategoryIdRole:
        return category.id();
    case NameRole:
        return category.name();
    case TypeRole:
        return Transaction::typeToString(category.type());
    case IsDefaultRole:
        return category.isDefault();
    case SortOrderRole:
        return category.sortOrder();
    default:
        return {};
    }
}

QHash<int, QByteArray> CategoryListModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {CategoryIdRole, "categoryId"},
        {NameRole, "name"},
        {TypeRole, "type"},
        {IsDefaultRole, "isDefault"},
        {SortOrderRole, "sortOrder"}
    };
}

void CategoryListModel::refresh(const QString &type)
{
    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(type, &typeOk);
    if (!typeOk) {
        m_hasCurrentType = false;
        setCategories({});
        return;
    }

    refresh(transactionType);
}

void CategoryListModel::refresh(TransactionType type)
{
    m_currentType = type;
    m_hasCurrentType = true;

    if (!m_categoryRepository) {
        setCategories({});
        return;
    }

    setCategories(m_categoryRepository->getCategoriesByType(type));
}

void CategoryListModel::refreshCurrent()
{
    if (!m_hasCurrentType) {
        return;
    }

    refresh(m_currentType);
}

void CategoryListModel::setCategoryRepository(CategoryRepository *categoryRepository)
{
    m_categoryRepository = categoryRepository;
}

void CategoryListModel::setCategories(const QList<Category> &categories)
{
    beginResetModel();
    m_categories = categories;
    endResetModel();
}
