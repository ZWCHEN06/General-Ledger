cmake_minimum_required(VERSION 3.16)

project(OpeningQtSmoke VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(opening_qt_smoke
    main.cpp
)

qt_add_qml_module(opening_qt_smoke
    URI OpeningQtSmoke
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(opening_qt_smoke
    PRIVATE Qt6::Quick
)

