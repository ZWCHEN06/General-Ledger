cmake_minimum_required(VERSION 3.16)

project(expense-tracker-qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Sql)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

qt_add_executable(expense-tracker-qt
    main.cpp
    src/database/DatabaseManager.cpp
    src/database/DatabaseManager.h
    src/models/Transaction.cpp
    src/models/Transaction.h
)

qt_add_qml_module(expense-tracker-qt
    URI ExpenseTracker
    VERSION 1.0
    QML_FILES
        Main.qml
        qml/pages/HomePage.qml
)

target_link_libraries(expense-tracker-qt
    PRIVATE
        Qt6::Quick
        Qt6::Sql
)
