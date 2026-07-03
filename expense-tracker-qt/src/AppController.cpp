#include "AppController.h"

#include "models/CategoryListModel.h"
#include "models/SubcategoryListModel.h"
#include "models/Transaction.h"
#include "models/TransactionFilter.h"
#include "models/TransactionListModel.h"
#include "models/WeeklyBudgetListModel.h"
#include "repositories/CategoryRepository.h"
#include "repositories/SubcategoryRepository.h"
#include "repositories/TransactionRepository.h"
#include "repositories/WeeklyBudgetRepository.h"
#include "services/CategorySummaryService.h"
#include "services/CsvExportService.h"
#include "services/SummaryService.h"
#include "services/WeeklyBudgetService.h"
#include "utils/WeekUtils.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QVariant>
#include <QVariantList>

#include <optional>

namespace {
QVariantMap failureResult(const QString &message)
{
    return QVariantMap {
        {QStringLiteral("success"), false},
        {QStringLiteral("errorMessage"), message},
        {QStringLiteral("id"), -1}
    };
}

QVariantMap successResult(bool filterActive = false)
{
    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("filterActive"), filterActive}
    };
}

QVariantMap categoryRepositoryResultToMap(const CategoryRepositoryResult &result)
{
    return QVariantMap {
        {QStringLiteral("success"), result.success},
        {QStringLiteral("errorMessage"), result.errorMessage},
        {QStringLiteral("id"), result.id}
    };
}

QVariantMap subcategoryRepositoryResultToMap(const SubcategoryRepositoryResult &result)
{
    return QVariantMap {
        {QStringLiteral("success"), result.success},
        {QStringLiteral("errorMessage"), result.errorMessage},
        {QStringLiteral("id"), result.id}
    };
}

QVariantMap transactionToMap(const Transaction &transaction)
{
    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), transaction.id()},
        {QStringLiteral("type"), Transaction::typeToString(transaction.type())},
        {QStringLiteral("amount"), transaction.amount()},
        {QStringLiteral("category"), transaction.category()},
        {QStringLiteral("categoryId"), transaction.categoryId().has_value()
             ? transaction.categoryId().value()
             : -1},
        {QStringLiteral("date"), transaction.date()},
        {QStringLiteral("note"), transaction.note()}
    };
}

bool isEmptyOptionalValue(const QVariant &value)
{
    return !value.isValid() || value.isNull() || value.toString().trimmed().isEmpty();
}

bool parseOptionalInt(const QVariant &value,
                      const QString &fieldName,
                      int minValue,
                      int maxValue,
                      std::optional<int> *output,
                      QString *errorMessage)
{
    if (isEmptyOptionalValue(value)) {
        return true;
    }

    bool ok = false;
    const int parsedValue = value.toString().trimmed().toInt(&ok);
    if (!ok) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("%1格式不正确").arg(fieldName);
        }
        return false;
    }

    if (parsedValue < minValue || parsedValue > maxValue) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("%1必须在 %2 到 %3 之间").arg(fieldName).arg(minValue).arg(maxValue);
        }
        return false;
    }

    if (output) {
        *output = parsedValue;
    }
    return true;
}

bool parseOptionalAmount(const QVariant &value,
                         const QString &fieldName,
                         std::optional<double> *output,
                         QString *errorMessage)
{
    if (isEmptyOptionalValue(value)) {
        return true;
    }

    bool ok = false;
    const double parsedValue = value.toString().trimmed().toDouble(&ok);
    if (!ok) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("%1格式不正确").arg(fieldName);
        }
        return false;
    }

    if (parsedValue < 0.0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("%1不能小于 0").arg(fieldName);
        }
        return false;
    }

    if (output) {
        *output = parsedValue;
    }
    return true;
}

bool parseRequiredBudgetAmount(const QVariant &value, double *output, QString *errorMessage)
{
    if (isEmptyOptionalValue(value)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("预算金额不能为空");
        }
        return false;
    }

    bool ok = false;
    const double parsedValue = value.toString().trimmed().toDouble(&ok);
    if (!ok) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("预算金额格式不正确");
        }
        return false;
    }

    if (parsedValue < 0.0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("预算金额不能小于 0");
        }
        return false;
    }

    if (output) {
        *output = parsedValue;
    }
    return true;
}

bool parseOptionalCategoryId(const QVariant &value, std::optional<int> *output, QString *errorMessage)
{
    if (output) {
        output->reset();
    }

    if (isEmptyOptionalValue(value)) {
        return true;
    }

    bool ok = false;
    const int parsedValue = value.toString().trimmed().toInt(&ok);
    if (!ok || parsedValue <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("分类ID无效");
        }
        return false;
    }

    if (output) {
        *output = parsedValue;
    }
    return true;
}

bool parseRequiredWeekStartDate(const QString &weekStartDate, QDate *output, QString *errorMessage)
{
    const QString trimmedWeekStartDate = weekStartDate.trimmed();
    const QDate parsedWeekStartDate = WeekUtils::parseDate(trimmedWeekStartDate);
    if (!parsedWeekStartDate.isValid()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("周开始日期必须是合法的 YYYY-MM-DD 格式");
        }
        return false;
    }

    if (!WeekUtils::isWeekStartDate(parsedWeekStartDate)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("周开始日期必须是周一");
        }
        return false;
    }

    if (output) {
        *output = parsedWeekStartDate;
    }
    return true;
}

bool parseOptionalTransactionType(const QString &type, TransactionFilter *filter, QString *errorMessage)
{
    const QString normalizedType = type.trimmed().toLower();
    if (normalizedType.isEmpty()
        || normalizedType == QStringLiteral("all")
        || normalizedType == QStringLiteral("全部")) {
        return true;
    }

    if (normalizedType == QStringLiteral("收入")) {
        if (filter) {
            filter->type = TransactionType::Income;
        }
        return true;
    }

    if (normalizedType == QStringLiteral("支出")) {
        if (filter) {
            filter->type = TransactionType::Expense;
        }
        return true;
    }

    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(normalizedType, &typeOk);
    if (!typeOk) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("账单类型不正确，请使用 income 或 expense");
        }
        return false;
    }

    if (filter) {
        filter->type = transactionType;
    }
    return true;
}

bool parseRequiredCategoryType(const QString &type, TransactionType *output, QString *errorMessage)
{
    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(type, &typeOk);
    if (!typeOk) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("分类类型必须是 income 或 expense");
        }
        return false;
    }

    if (output) {
        *output = transactionType;
    }
    return true;
}

QString optionalDoubleToString(const std::optional<double> &value)
{
    if (!value.has_value()) {
        return QString();
    }

    return QString::number(value.value(), 'g', 15);
}

}

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

AppController::AppController(TransactionRepository *transactionRepository, QObject *parent)
    : QObject(parent),
      m_transactionRepository(transactionRepository)
{
}

bool AppController::databaseReady() const
{
    return m_databaseReady;
}

QString AppController::databaseErrorMessage() const
{
    return effectiveDatabaseErrorMessage();
}

bool AppController::transactionFilterActive() const
{
    return m_transactionFilterActive;
}

SubcategoryListModel *AppController::subcategoryListModel() const
{
    return m_subcategoryListModel;
}

WeeklyBudgetListModel *AppController::weeklyBudgetListModel() const
{
    return m_weeklyBudgetListModel;
}

double AppController::totalBudget() const
{
    return m_weeklyBudgetSummary.totalBudget;
}

double AppController::totalActual() const
{
    return m_weeklyBudgetSummary.totalActual;
}

double AppController::totalRemaining() const
{
    return m_weeklyBudgetSummary.totalRemaining;
}

double AppController::totalUsagePercent() const
{
    return m_weeklyBudgetSummary.totalUsagePercent;
}

bool AppController::isTotalOverBudget() const
{
    return m_weeklyBudgetSummary.isTotalOverBudget;
}

void AppController::setDatabaseStatus(bool ready, const QString &errorMessage)
{
    if (m_databaseReady == ready && m_databaseErrorMessage == errorMessage) {
        return;
    }

    m_databaseReady = ready;
    m_databaseErrorMessage = errorMessage;
    emit databaseStatusChanged();
}

void AppController::setTransactionListModel(TransactionListModel *transactionListModel)
{
    m_transactionListModel = transactionListModel;
}

void AppController::setCategoryRepository(CategoryRepository *categoryRepository)
{
    m_categoryRepository = categoryRepository;
}

void AppController::setCategoryListModel(CategoryListModel *categoryListModel)
{
    m_categoryListModel = categoryListModel;
}

void AppController::setSubcategoryRepository(SubcategoryRepository *subcategoryRepository)
{
    m_subcategoryRepository = subcategoryRepository;
}

void AppController::setSubcategoryListModel(SubcategoryListModel *subcategoryListModel)
{
    m_subcategoryListModel = subcategoryListModel;
}

void AppController::setWeeklyBudgetListModel(WeeklyBudgetListModel *weeklyBudgetListModel)
{
    m_weeklyBudgetListModel = weeklyBudgetListModel;
}

void AppController::setWeeklyBudgetRepository(WeeklyBudgetRepository *weeklyBudgetRepository)
{
    m_weeklyBudgetRepository = weeklyBudgetRepository;
}

QString AppController::effectiveDatabaseErrorMessage() const
{
    if (!m_databaseErrorMessage.trimmed().isEmpty()) {
        return m_databaseErrorMessage;
    }

    return QStringLiteral("数据库不可用，请检查 SQLite 打开和初始化日志");
}

QString AppController::testMessage() const
{
    return QStringLiteral("C++ 已连接");
}

QVariantMap AppController::addTransaction(const QString &type,
                                          const QString &amount,
                                          const QString &category,
                                          const QString &date,
                                          const QString &note)
{
    return addTransaction(type, amount, category, date, note, QVariant());
}

QVariantMap AppController::addTransaction(const QString &type,
                                          const QString &amount,
                                          const QString &category,
                                          const QString &date,
                                          const QString &note,
                                          const QVariant &categoryId)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(type, &typeOk);
    if (!typeOk) {
        return failureResult(QStringLiteral("账单类型不正确"));
    }

    bool amountOk = false;
    const double transactionAmount = amount.trimmed().toDouble(&amountOk);
    if (!amountOk) {
        return failureResult(QStringLiteral("金额格式不正确"));
    }

    std::optional<int> parsedCategoryId;
    QString categoryIdError;
    if (!parseOptionalCategoryId(categoryId, &parsedCategoryId, &categoryIdError)) {
        return failureResult(categoryIdError);
    }

    if (parsedCategoryId.has_value()) {
        if (!m_categoryRepository) {
            return failureResult(QStringLiteral("分类仓库未初始化"));
        }

        bool categoryExists = false;
        const QList<Category> categories = m_categoryRepository->getCategoriesByType(transactionType);
        for (const Category &existingCategory : categories) {
            if (existingCategory.id() == parsedCategoryId.value()) {
                categoryExists = true;
                break;
            }
        }

        if (!categoryExists) {
            return failureResult(QStringLiteral("分类ID无效"));
        }
    }

    Transaction transaction(
        0,
        transactionType,
        transactionAmount,
        category.trimmed(),
        date.trimmed(),
        note.trimmed(),
        QString(),
        QString(),
        parsedCategoryId);

    QString validationError;
    if (!transaction.validate(&validationError)) {
        return failureResult(validationError);
    }

    const int id = m_transactionRepository->addTransaction(transaction);
    if (id <= 0) {
        return failureResult(QStringLiteral("保存账单失败，请稍后重试"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), id}
    };
}

QVariantMap AppController::getTransactionById(int id) const
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    const std::optional<Transaction> transaction = m_transactionRepository->getTransactionById(id);
    if (!transaction.has_value()) {
        return failureResult(QStringLiteral("未找到账单记录"));
    }

    return transactionToMap(transaction.value());
}

QVariantMap AppController::updateTransaction(int id,
                                             const QString &type,
                                             const QString &amount,
                                             const QString &category,
                                             const QString &date,
                                             const QString &note)
{
    return updateTransaction(id, type, amount, category, date, note, QVariant());
}

QVariantMap AppController::updateTransaction(int id,
                                             const QString &type,
                                             const QString &amount,
                                             const QString &category,
                                             const QString &date,
                                             const QString &note,
                                             const QVariant &categoryId)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    bool typeOk = false;
    const TransactionType transactionType = Transaction::typeFromString(type, &typeOk);
    if (!typeOk) {
        return failureResult(QStringLiteral("账单类型不正确"));
    }

    bool amountOk = false;
    const double transactionAmount = amount.trimmed().toDouble(&amountOk);
    if (!amountOk) {
        return failureResult(QStringLiteral("金额格式不正确"));
    }

    std::optional<int> parsedCategoryId;
    QString categoryIdError;
    if (!parseOptionalCategoryId(categoryId, &parsedCategoryId, &categoryIdError)) {
        return failureResult(categoryIdError);
    }

    if (parsedCategoryId.has_value()) {
        if (!m_categoryRepository) {
            return failureResult(QStringLiteral("分类仓库未初始化"));
        }

        bool categoryExists = false;
        const QList<Category> categories = m_categoryRepository->getCategoriesByType(transactionType);
        for (const Category &existingCategory : categories) {
            if (existingCategory.id() == parsedCategoryId.value()) {
                categoryExists = true;
                break;
            }
        }

        if (!categoryExists) {
            return failureResult(QStringLiteral("分类ID无效"));
        }
    }

    Transaction transaction(
        id,
        transactionType,
        transactionAmount,
        category.trimmed(),
        date.trimmed(),
        note.trimmed(),
        QString(),
        QString(),
        parsedCategoryId);

    QString validationError;
    if (!transaction.validate(&validationError)) {
        return failureResult(validationError);
    }

    if (!m_transactionRepository->updateTransaction(transaction)) {
        return failureResult(QStringLiteral("更新账单失败，请确认记录是否存在"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), id}
    };
}

QVariantMap AppController::deleteTransaction(int id)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    if (!m_transactionRepository->deleteTransaction(id)) {
        return failureResult(QStringLiteral("删除账单失败，请确认记录是否存在"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), id}
    };
}

QVariantMap AppController::currentMonthSummary() const
{
    if (!m_databaseReady) {
        return QVariantMap {
            {QStringLiteral("success"), false},
            {QStringLiteral("errorMessage"), effectiveDatabaseErrorMessage()},
            {QStringLiteral("income"), 0.0},
            {QStringLiteral("expense"), 0.0},
            {QStringLiteral("balance"), 0.0}
        };
    }

    if (!m_transactionRepository) {
        return QVariantMap {
            {QStringLiteral("success"), false},
            {QStringLiteral("errorMessage"), QStringLiteral("账单仓库未初始化")},
            {QStringLiteral("income"), 0.0},
            {QStringLiteral("expense"), 0.0},
            {QStringLiteral("balance"), 0.0}
        };
    }

    const QDate currentDate = QDate::currentDate();
    const QList<Transaction> transactions =
        m_transactionRepository->getTransactionsByMonth(currentDate.year(), currentDate.month());

    const SummaryService summaryService;
    const SummaryResult summary = summaryService.calculate(transactions);

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("income"), summary.totalIncome},
        {QStringLiteral("expense"), summary.totalExpense},
        {QStringLiteral("balance"), summary.balance}
    };
}

QVariantMap AppController::exportCsv() const
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    const QList<Transaction> transactions = m_transactionRepository->getAllTransactions();
    if (transactions.isEmpty()) {
        return failureResult(QStringLiteral("暂无可导出的账单"));
    }

    const CsvExportService csvExportService;
    const QString csvContent = csvExportService.exportTransactions(transactions);

#ifdef Q_OS_ANDROID
    Q_UNUSED(csvContent)
    return failureResult(QStringLiteral("当前 Android 版暂未接入系统文件保存或分享能力，无法导出 CSV"));
#else
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documentsPath.trimmed().isEmpty()) {
        documentsPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    if (documentsPath.trimmed().isEmpty()) {
        return failureResult(QStringLiteral("无法获取可写入的导出目录"));
    }

    QDir documentsDir(documentsPath);
    if (!documentsDir.exists() && !QDir().mkpath(documentsPath)) {
        return failureResult(QStringLiteral("无法创建导出目录：%1").arg(documentsPath));
    }

    const QString fileName = QStringLiteral("expense_transactions_%1.csv")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString filePath = documentsDir.filePath(fileName);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return failureResult(QStringLiteral("无法创建 CSV 文件：%1").arg(file.errorString()));
    }

    QByteArray output;
    output.append("\xEF\xBB\xBF", 3);
    output.append(csvContent.toUtf8());

    const qint64 bytesWritten = file.write(output);
    file.close();

    if (bytesWritten != output.size()) {
        QFile::remove(filePath);
        return failureResult(QStringLiteral("写入 CSV 文件失败"));
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("filePath"), filePath}
    };
#endif
}

QVariantMap AppController::currentMonthCategorySummary() const
{
    if (!m_databaseReady) {
        return QVariantMap {
            {QStringLiteral("success"), false},
            {QStringLiteral("errorMessage"), effectiveDatabaseErrorMessage()},
            {QStringLiteral("items"), QVariantList()}
        };
    }

    if (!m_transactionRepository) {
        return QVariantMap {
            {QStringLiteral("success"), false},
            {QStringLiteral("errorMessage"), QStringLiteral("账单仓库未初始化")},
            {QStringLiteral("items"), QVariantList()}
        };
    }

    const QDate currentDate = QDate::currentDate();
    const QList<Transaction> transactions =
        m_transactionRepository->getTransactionsByMonth(currentDate.year(), currentDate.month());

    const CategorySummaryService categorySummaryService;
    const QList<CategorySummaryItem> summaryItems = categorySummaryService.calculate(transactions);

    QVariantList items;
    items.reserve(summaryItems.size());
    for (const CategorySummaryItem &summaryItem : summaryItems) {
        items.append(QVariantMap {
            {QStringLiteral("category"), summaryItem.category},
            {QStringLiteral("amount"), summaryItem.amount}
        });
    }

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("items"), items}
    };
}

QVariantMap AppController::applyTransactionFilter(const QVariant &year,
                                                  const QVariant &month,
                                                  const QString &type,
                                                  const QString &category,
                                                  const QString &keyword,
                                                  const QVariant &minAmount,
                                                  const QVariant &maxAmount)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionListModel) {
        return failureResult(QStringLiteral("账单列表模型未初始化"));
    }

    TransactionFilter filter;
    QString validationError;

    if (!parseOptionalInt(year, QStringLiteral("年份"), 1, 9999, &filter.year, &validationError)) {
        return failureResult(validationError);
    }

    if (!parseOptionalInt(month, QStringLiteral("月份"), 1, 12, &filter.month, &validationError)) {
        return failureResult(validationError);
    }

    if (!parseOptionalTransactionType(type, &filter, &validationError)) {
        return failureResult(validationError);
    }

    const QString trimmedCategory = category.trimmed();
    if (!trimmedCategory.isEmpty()) {
        filter.category = trimmedCategory;
    }

    const QString trimmedKeyword = keyword.trimmed();
    if (!trimmedKeyword.isEmpty()) {
        filter.keyword = trimmedKeyword;
    }

    if (!parseOptionalAmount(minAmount, QStringLiteral("最小金额"), &filter.minAmount, &validationError)) {
        return failureResult(validationError);
    }

    if (!parseOptionalAmount(maxAmount, QStringLiteral("最大金额"), &filter.maxAmount, &validationError)) {
        return failureResult(validationError);
    }

    if (!filter.validate(&validationError)) {
        return failureResult(validationError);
    }

    m_transactionFilter = filter;
    m_transactionFilterActive = m_transactionFilter.hasActiveConditions();

    if (m_transactionFilterActive) {
        m_transactionListModel->refreshWithFilter(m_transactionFilter);
    } else {
        m_transactionListModel->refresh();
    }

    emit transactionFilterChanged();
    return successResult(m_transactionFilterActive);
}

QVariantMap AppController::clearTransactionFilter()
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionListModel) {
        return failureResult(QStringLiteral("账单列表模型未初始化"));
    }

    m_transactionFilter = TransactionFilter();
    m_transactionFilterActive = false;

    m_transactionListModel->refresh();
    emit transactionFilterChanged();
    return successResult(false);
}

QVariantMap AppController::currentTransactionFilter() const
{
    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("filterActive"), m_transactionFilterActive},
        {QStringLiteral("year"), m_transactionFilter.year.has_value()
             ? QString::number(m_transactionFilter.year.value())
             : QString()},
        {QStringLiteral("month"), m_transactionFilter.month.has_value()
             ? QString::number(m_transactionFilter.month.value())
             : QString()},
        {QStringLiteral("type"), m_transactionFilter.type.has_value()
             ? Transaction::typeToString(m_transactionFilter.type.value())
             : QStringLiteral("all")},
        {QStringLiteral("category"), m_transactionFilter.category.value_or(QString())},
        {QStringLiteral("keyword"), m_transactionFilter.keyword.value_or(QString())},
        {QStringLiteral("minAmount"), optionalDoubleToString(m_transactionFilter.minAmount)},
        {QStringLiteral("maxAmount"), optionalDoubleToString(m_transactionFilter.maxAmount)}
    };
}

QVariantMap AppController::refreshTransactionList()
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_transactionListModel) {
        return failureResult(QStringLiteral("账单列表模型未初始化"));
    }

    if (m_transactionFilterActive) {
        m_transactionListModel->refreshWithFilter(m_transactionFilter);
    } else {
        m_transactionListModel->refresh();
    }

    return successResult(m_transactionFilterActive);
}

QVariantMap AppController::refreshCategories(const QString &type)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_categoryListModel) {
        return failureResult(QStringLiteral("分类列表模型未初始化"));
    }

    TransactionType categoryType = TransactionType::Expense;
    QString validationError;
    if (!parseRequiredCategoryType(type, &categoryType, &validationError)) {
        return failureResult(validationError);
    }

    m_categoryListModel->refresh(categoryType);
    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), -1}
    };
}

QVariantMap AppController::addCategory(const QString &name, const QString &type)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_categoryRepository) {
        return failureResult(QStringLiteral("分类仓库未初始化"));
    }

    if (!m_categoryListModel) {
        return failureResult(QStringLiteral("分类列表模型未初始化"));
    }

    TransactionType categoryType = TransactionType::Expense;
    QString validationError;
    if (!parseRequiredCategoryType(type, &categoryType, &validationError)) {
        return failureResult(validationError);
    }

    const CategoryRepositoryResult result = m_categoryRepository->addCategory(name, categoryType);
    if (result.success) {
        m_categoryListModel->refresh(categoryType);
    }

    return categoryRepositoryResultToMap(result);
}

QVariantMap AppController::updateCategory(int id, const QString &name)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_categoryRepository) {
        return failureResult(QStringLiteral("分类仓库未初始化"));
    }

    if (!m_categoryListModel) {
        return failureResult(QStringLiteral("分类列表模型未初始化"));
    }

    const CategoryRepositoryResult result = m_categoryRepository->updateCategoryName(id, name);
    if (result.success) {
        m_categoryListModel->refreshCurrent();
    }

    return categoryRepositoryResultToMap(result);
}

QVariantMap AppController::deleteCategory(int id)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_categoryRepository) {
        return failureResult(QStringLiteral("分类仓库未初始化"));
    }

    if (!m_categoryListModel) {
        return failureResult(QStringLiteral("分类列表模型未初始化"));
    }

    const CategoryRepositoryResult result = m_categoryRepository->deleteCategory(id);
    if (result.success) {
        m_categoryListModel->refreshCurrent();
    }

    return categoryRepositoryResultToMap(result);
}

QVariantMap AppController::refreshSubcategories(int categoryId)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_subcategoryRepository) {
        return failureResult(QStringLiteral("二级分类仓库未初始化"));
    }

    if (!m_subcategoryListModel) {
        return failureResult(QStringLiteral("二级分类列表模型未初始化"));
    }

    if (categoryId <= 0) {
        return failureResult(QStringLiteral("一级分类 id 必须大于 0"));
    }

    m_subcategoryListModel->refresh(categoryId);
    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("id"), categoryId}
    };
}

QVariantMap AppController::addSubcategory(int categoryId, const QString &name)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_subcategoryRepository) {
        return failureResult(QStringLiteral("二级分类仓库未初始化"));
    }

    if (!m_subcategoryListModel) {
        return failureResult(QStringLiteral("二级分类列表模型未初始化"));
    }

    const SubcategoryRepositoryResult result = m_subcategoryRepository->addSubcategory(categoryId, name);
    if (result.success) {
        m_subcategoryListModel->refresh(categoryId);
    }

    return subcategoryRepositoryResultToMap(result);
}

QVariantMap AppController::updateSubcategory(int id, const QString &name)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_subcategoryRepository) {
        return failureResult(QStringLiteral("二级分类仓库未初始化"));
    }

    if (!m_subcategoryListModel) {
        return failureResult(QStringLiteral("二级分类列表模型未初始化"));
    }

    const QList<Subcategory> subcategories = m_subcategoryRepository->getSubcategoryById(id);
    if (subcategories.isEmpty()) {
        return failureResult(QStringLiteral("二级分类不存在"));
    }

    const int categoryId = subcategories.first().categoryId();
    const SubcategoryRepositoryResult result = m_subcategoryRepository->updateSubcategoryName(id, name);
    if (result.success) {
        m_subcategoryListModel->refresh(categoryId);
    }

    return subcategoryRepositoryResultToMap(result);
}

QVariantMap AppController::deleteSubcategory(int id)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_subcategoryRepository) {
        return failureResult(QStringLiteral("二级分类仓库未初始化"));
    }

    if (!m_subcategoryListModel) {
        return failureResult(QStringLiteral("二级分类列表模型未初始化"));
    }

    const QList<Subcategory> subcategories = m_subcategoryRepository->getSubcategoryById(id);
    if (subcategories.isEmpty()) {
        return failureResult(QStringLiteral("二级分类不存在"));
    }

    const int categoryId = subcategories.first().categoryId();
    const SubcategoryRepositoryResult result = m_subcategoryRepository->deleteSubcategory(id);
    if (result.success) {
        m_subcategoryListModel->refresh(categoryId);
    }

    return subcategoryRepositoryResultToMap(result);
}

QVariantMap AppController::loadWeeklyBudget(const QString &weekStartDate)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_categoryRepository) {
        return failureResult(QStringLiteral("分类仓库未初始化"));
    }

    if (!m_transactionRepository) {
        return failureResult(QStringLiteral("账单仓库未初始化"));
    }

    if (!m_weeklyBudgetRepository) {
        return failureResult(QStringLiteral("每周预算仓库未初始化"));
    }

    if (!m_weeklyBudgetListModel) {
        return failureResult(QStringLiteral("每周预算列表模型未初始化"));
    }

    const QString trimmedWeekStartDate = weekStartDate.trimmed();
    QDate parsedWeekStartDate;
    QString validationError;
    if (!parseRequiredWeekStartDate(trimmedWeekStartDate, &parsedWeekStartDate, &validationError)) {
        return failureResult(validationError);
    }

    const QDate weekEndDate = WeekUtils::getWeekEndDate(parsedWeekStartDate);
    const QString formattedWeekEndDate = WeekUtils::formatDate(weekEndDate);

    const QList<Category> expenseCategories = m_categoryRepository->getCategoriesByType(TransactionType::Expense);
    const QList<WeeklyBudget> budgets = m_weeklyBudgetRepository->getBudgetsByWeek(trimmedWeekStartDate);
    const QHash<int, double> actualExpensesByCategory =
        m_transactionRepository->getWeeklyExpenseByCategory(trimmedWeekStartDate, formattedWeekEndDate);

    const WeeklyBudgetService weeklyBudgetService;
    const QList<WeeklyBudgetComparisonItem> items =
        weeklyBudgetService.calculate(expenseCategories, budgets, actualExpensesByCategory);
    const WeeklyBudgetSummary summary = weeklyBudgetService.summarize(items);

    m_weeklyBudgetListModel->setItems(items);
    m_weeklyBudgetSummary = summary;
    emit weeklyBudgetSummaryChanged();

    return QVariantMap {
        {QStringLiteral("success"), true},
        {QStringLiteral("errorMessage"), QString()},
        {QStringLiteral("weekStartDate"), trimmedWeekStartDate},
        {QStringLiteral("weekEndDate"), formattedWeekEndDate},
        {QStringLiteral("totalBudget"), m_weeklyBudgetSummary.totalBudget},
        {QStringLiteral("totalActual"), m_weeklyBudgetSummary.totalActual},
        {QStringLiteral("totalRemaining"), m_weeklyBudgetSummary.totalRemaining},
        {QStringLiteral("totalUsagePercent"), m_weeklyBudgetSummary.totalUsagePercent},
        {QStringLiteral("isTotalOverBudget"), m_weeklyBudgetSummary.isTotalOverBudget}
    };
}

QVariantMap AppController::setWeeklyBudget(const QString &weekStartDate,
                                           int categoryId,
                                           const QVariant &amount)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_categoryRepository) {
        return failureResult(QStringLiteral("分类仓库未初始化"));
    }

    if (!m_weeklyBudgetRepository) {
        return failureResult(QStringLiteral("每周预算仓库未初始化"));
    }

    const QString trimmedWeekStartDate = weekStartDate.trimmed();
    QDate parsedWeekStartDate;
    QString validationError;
    if (!parseRequiredWeekStartDate(trimmedWeekStartDate, &parsedWeekStartDate, &validationError)) {
        return failureResult(validationError);
    }

    if (categoryId <= 0) {
        return failureResult(QStringLiteral("分类 id 必须大于 0"));
    }

    double parsedAmount = 0.0;
    if (!parseRequiredBudgetAmount(amount, &parsedAmount, &validationError)) {
        return failureResult(validationError);
    }

    bool expenseCategoryExists = false;
    const QList<Category> expenseCategories = m_categoryRepository->getCategoriesByType(TransactionType::Expense);
    for (const Category &category : expenseCategories) {
        if (category.id() == categoryId) {
            expenseCategoryExists = true;
            break;
        }
    }

    if (!expenseCategoryExists) {
        return failureResult(QStringLiteral("只能为支出分类设置预算"));
    }

    const WeeklyBudgetRepositoryResult result =
        m_weeklyBudgetRepository->upsertBudget(trimmedWeekStartDate, categoryId, parsedAmount);
    if (!result.success) {
        return failureResult(result.errorMessage);
    }

    QVariantMap loadResult = loadWeeklyBudget(trimmedWeekStartDate);
    if (!loadResult.value(QStringLiteral("success")).toBool()) {
        return loadResult;
    }

    loadResult.insert(QStringLiteral("id"), result.id);
    return loadResult;
}

QVariantMap AppController::deleteWeeklyBudget(const QString &weekStartDate, int categoryId)
{
    if (!m_databaseReady) {
        return failureResult(effectiveDatabaseErrorMessage());
    }

    if (!m_weeklyBudgetRepository) {
        return failureResult(QStringLiteral("每周预算仓库未初始化"));
    }

    const QString trimmedWeekStartDate = weekStartDate.trimmed();
    QDate parsedWeekStartDate;
    QString validationError;
    if (!parseRequiredWeekStartDate(trimmedWeekStartDate, &parsedWeekStartDate, &validationError)) {
        return failureResult(validationError);
    }

    if (categoryId <= 0) {
        return failureResult(QStringLiteral("分类 id 必须大于 0"));
    }

    const WeeklyBudgetRepositoryResult result =
        m_weeklyBudgetRepository->deleteBudget(trimmedWeekStartDate, categoryId);
    if (!result.success) {
        return failureResult(result.errorMessage);
    }

    QVariantMap loadResult = loadWeeklyBudget(trimmedWeekStartDate);
    if (!loadResult.value(QStringLiteral("success")).toBool()) {
        return loadResult;
    }

    loadResult.insert(QStringLiteral("id"), result.id);
    return loadResult;
}
