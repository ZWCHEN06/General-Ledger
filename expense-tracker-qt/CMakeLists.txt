cmake_minimum_required(VERSION 3.16)

project(expense-tracker-qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Sql)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

qt_add_executable(expense-tracker-qt
    main.cpp
    src/AppController.cpp
    src/AppController.h
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/models/Category.cpp
    src/models/Category.h
    src/models/CategoryListModel.cpp
    src/models/CategoryListModel.h
    src/models/Transaction.cpp
    src/models/Transaction.h
    src/models/TransactionFilter.cpp
    src/models/TransactionFilter.h
    src/models/TransactionListModel.cpp
    src/models/TransactionListModel.h
    src/models/WeeklyBudget.cpp
    src/models/WeeklyBudget.h
    src/repositories/CategoryRepository.cpp
    src/repositories/CategoryRepository.h
    src/repositories/TransactionRepository.cpp
    src/repositories/TransactionRepository.h
    src/repositories/WeeklyBudgetRepository.cpp
    src/repositories/WeeklyBudgetRepository.h
    src/services/CategorySummaryService.cpp
    src/services/CategorySummaryService.h
    src/services/CsvExportService.cpp
    src/services/CsvExportService.h
    src/services/SummaryService.cpp
    src/services/SummaryService.h
    src/utils/WeekUtils.cpp
    src/utils/WeekUtils.h
)

qt_add_qml_module(expense-tracker-qt
    URI ExpenseTracker
    VERSION 1.0
    QML_FILES
        Main.qml
        qml/components/CategoryItem.qml
        qml/components/CategoryOption.qml
        qml/components/FormField.qml
        qml/components/TypeOption.qml
        qml/pages/AddTransactionPage.qml
        qml/pages/CategoryManagePage.qml
        qml/pages/CategorySummaryPage.qml
        qml/pages/EditTransactionPage.qml
        qml/pages/HomePage.qml
        qml/pages/SettingsPage.qml
        qml/pages/TransactionListPage.qml
)

target_link_libraries(expense-tracker-qt
    PRIVATE
        Qt6::Quick
        Qt6::Sql
)
