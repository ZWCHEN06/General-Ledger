#pragma once

#include "Category.h"

#include <QAbstractListModel>
#include <QList>
#include <QString>

class CategoryRepository;

class CategoryListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        CategoryIdRole,
        NameRole,
        TypeRole,
        IsDefaultRole,
        SortOrderRole
    };

    explicit CategoryListModel(QObject *parent = nullptr);
    explicit CategoryListModel(CategoryRepository *categoryRepository, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh(const QString &type);
    void refresh(TransactionType type);
    void refreshCurrent();

    void setCategoryRepository(CategoryRepository *categoryRepository);
    void setCategories(const QList<Category> &categories);

private:
    CategoryRepository *m_categoryRepository = nullptr;
    QList<Category> m_categories;
    TransactionType m_currentType = TransactionType::Expense;
    bool m_hasCurrentType = false;
};
