#pragma once

#include "models/TransactionFilter.h"
#include "models/WeeklyBudgetListModel.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

class CategoryListModel;
class CategoryRepository;
class TransactionListModel;
class TransactionRepository;
class WeeklyBudgetRepository;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool databaseReady READ databaseReady NOTIFY databaseStatusChanged)
    Q_PROPERTY(QString databaseErrorMessage READ databaseErrorMessage NOTIFY databaseStatusChanged)
    Q_PROPERTY(bool transactionFilterActive READ transactionFilterActive NOTIFY transactionFilterChanged)
    Q_PROPERTY(WeeklyBudgetListModel* weeklyBudgetListModel READ weeklyBudgetListModel CONSTANT)
    Q_PROPERTY(double totalBudget READ totalBudget NOTIFY weeklyBudgetSummaryChanged)
    Q_PROPERTY(double totalActual READ totalActual NOTIFY weeklyBudgetSummaryChanged)
    Q_PROPERTY(double totalRemaining READ totalRemaining NOTIFY weeklyBudgetSummaryChanged)
    Q_PROPERTY(double totalUsagePercent READ totalUsagePercent NOTIFY weeklyBudgetSummaryChanged)
    Q_PROPERTY(bool isTotalOverBudget READ isTotalOverBudget NOTIFY weeklyBudgetSummaryChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    explicit AppController(TransactionRepository *transactionRepository, QObject *parent = nullptr);

    bool databaseReady() const;
    QString databaseErrorMessage() const;
    bool transactionFilterActive() const;
    WeeklyBudgetListModel *weeklyBudgetListModel() const;
    double totalBudget() const;
    double totalActual() const;
    double totalRemaining() const;
    double totalUsagePercent() const;
    bool isTotalOverBudget() const;
    void setDatabaseStatus(bool ready, const QString &errorMessage);

    Q_INVOKABLE QString testMessage() const;
    Q_INVOKABLE QVariantMap addTransaction(const QString &type,
                                           const QString &amount,
                                           const QString &category,
                                           const QString &date,
                                           const QString &note);
    Q_INVOKABLE QVariantMap addTransaction(const QString &type,
                                           const QString &amount,
                                           const QString &category,
                                           const QString &date,
                                           const QString &note,
                                           const QVariant &categoryId);
    Q_INVOKABLE QVariantMap currentMonthSummary() const;
    Q_INVOKABLE QVariantMap getTransactionById(int id) const;
    Q_INVOKABLE QVariantMap updateTransaction(int id,
                                              const QString &type,
                                              const QString &amount,
                                              const QString &category,
                                              const QString &date,
                                              const QString &note);
    Q_INVOKABLE QVariantMap updateTransaction(int id,
                                              const QString &type,
                                              const QString &amount,
                                              const QString &category,
                                              const QString &date,
                                              const QString &note,
                                              const QVariant &categoryId);
    Q_INVOKABLE QVariantMap deleteTransaction(int id);
    Q_INVOKABLE QVariantMap exportCsv() const;
    Q_INVOKABLE QVariantMap currentMonthCategorySummary() const;
    Q_INVOKABLE QVariantMap applyTransactionFilter(const QVariant &year,
                                                   const QVariant &month,
                                                   const QString &type,
                                                   const QString &category,
                                                   const QString &keyword,
                                                   const QVariant &minAmount,
                                                   const QVariant &maxAmount);
    Q_INVOKABLE QVariantMap clearTransactionFilter();
    Q_INVOKABLE QVariantMap currentTransactionFilter() const;
    Q_INVOKABLE QVariantMap refreshTransactionList();
    Q_INVOKABLE QVariantMap refreshCategories(const QString &type);
    Q_INVOKABLE QVariantMap addCategory(const QString &name, const QString &type);
    Q_INVOKABLE QVariantMap updateCategory(int id, const QString &name);
    Q_INVOKABLE QVariantMap deleteCategory(int id);
    Q_INVOKABLE QVariantMap loadWeeklyBudget(const QString &weekStartDate);

    void setTransactionListModel(TransactionListModel *transactionListModel);
    void setCategoryRepository(CategoryRepository *categoryRepository);
    void setCategoryListModel(CategoryListModel *categoryListModel);
    void setWeeklyBudgetListModel(WeeklyBudgetListModel *weeklyBudgetListModel);
    void setWeeklyBudgetRepository(WeeklyBudgetRepository *weeklyBudgetRepository);

signals:
    void databaseStatusChanged();
    void transactionFilterChanged();
    void weeklyBudgetSummaryChanged();

private:
    QString effectiveDatabaseErrorMessage() const;

    TransactionRepository *m_transactionRepository = nullptr;
    TransactionListModel *m_transactionListModel = nullptr;
    CategoryRepository *m_categoryRepository = nullptr;
    CategoryListModel *m_categoryListModel = nullptr;
    WeeklyBudgetListModel *m_weeklyBudgetListModel = nullptr;
    WeeklyBudgetRepository *m_weeklyBudgetRepository = nullptr;
    WeeklyBudgetSummary m_weeklyBudgetSummary;
    TransactionFilter m_transactionFilter;
    bool m_transactionFilterActive = false;
    bool m_databaseReady = true;
    QString m_databaseErrorMessage;
};
