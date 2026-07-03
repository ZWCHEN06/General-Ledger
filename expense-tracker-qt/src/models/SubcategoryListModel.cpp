#include "SubcategoryListModel.h"

#include "../repositories/SubcategoryRepository.h"

SubcategoryListModel::SubcategoryListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

SubcategoryListModel::SubcategoryListModel(SubcategoryRepository *subcategoryRepository, QObject *parent)
    : QAbstractListModel(parent),
      m_subcategoryRepository(subcategoryRepository)
{
}

int SubcategoryListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_subcategories.size();
}

QVariant SubcategoryListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_subcategories.size()) {
        return {};
    }

    const Subcategory &subcategory = m_subcategories.at(index.row());

    switch (role) {
    case IdRole:
        return subcategory.id();
    case CategoryIdRole:
        return subcategory.categoryId();
    case NameRole:
        return subcategory.name();
    case IsDefaultRole:
        return subcategory.isDefault();
    case SortOrderRole:
        return subcategory.sortOrder();
    default:
        return {};
    }
}

QHash<int, QByteArray> SubcategoryListModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {CategoryIdRole, "categoryId"},
        {NameRole, "name"},
        {IsDefaultRole, "isDefault"},
        {SortOrderRole, "sortOrder"}
    };
}

void SubcategoryListModel::refresh(int categoryId)
{
    if (!m_subcategoryRepository || categoryId <= 0) {
        clear();
        return;
    }

    setSubcategories(m_subcategoryRepository->getSubcategoriesByCategoryId(categoryId));
}

void SubcategoryListModel::clear()
{
    setSubcategories({});
}

void SubcategoryListModel::setSubcategoryRepository(SubcategoryRepository *subcategoryRepository)
{
    m_subcategoryRepository = subcategoryRepository;
}

void SubcategoryListModel::setSubcategories(const QList<Subcategory> &subcategories)
{
    beginResetModel();
    m_subcategories = subcategories;
    endResetModel();
}
