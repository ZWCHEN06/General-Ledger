#pragma once

#include "Subcategory.h"

#include <QAbstractListModel>
#include <QList>

class SubcategoryRepository;

class SubcategoryListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        SubcategoryIdRole,
        CategoryIdRole,
        NameRole,
        IsDefaultRole,
        SortOrderRole
    };

    explicit SubcategoryListModel(QObject *parent = nullptr);
    explicit SubcategoryListModel(SubcategoryRepository *subcategoryRepository, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh(int categoryId);
    Q_INVOKABLE void clear();

    void setSubcategoryRepository(SubcategoryRepository *subcategoryRepository);
    void setSubcategories(const QList<Subcategory> &subcategories);

private:
    SubcategoryRepository *m_subcategoryRepository = nullptr;
    QList<Subcategory> m_subcategories;
};
