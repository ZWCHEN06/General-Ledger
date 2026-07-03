pragma ComponentBehavior: Bound

import QtQuick
import "qml/pages"

Window {
    id: root

    property int editingTransactionId: 0
    property int managingSubcategoryCategoryId: -1
    property string managingSubcategoryCategoryName: ""

    width: 360
    height: 640
    visible: true
    title: "记账 App"
    color: "#f7f7f7"

    Loader {
        id: pageLoader

        anchors.fill: parent
        sourceComponent: homePageComponent
    }

    Component {
        id: homePageComponent

        HomePage {
            onAddTransactionRequested: pageLoader.sourceComponent = addTransactionPageComponent
            onTransactionListRequested: pageLoader.sourceComponent = transactionListPageComponent
            onCategorySummaryRequested: pageLoader.sourceComponent = categorySummaryPageComponent
            onWeeklyBudgetRequested: pageLoader.sourceComponent = weeklyBudgetPageComponent
            onSettingsRequested: pageLoader.sourceComponent = settingsPageComponent
        }
    }

    Component {
        id: addTransactionPageComponent

        AddTransactionPage {
            onBackRequested: pageLoader.sourceComponent = homePageComponent
            onTransactionSaved: pageLoader.sourceComponent = homePageComponent
        }
    }

    Component {
        id: transactionListPageComponent

        TransactionListPage {
            onBackRequested: pageLoader.sourceComponent = homePageComponent
            onEditTransactionRequested: function(transactionId) {
                root.editingTransactionId = transactionId
                pageLoader.sourceComponent = editTransactionPageComponent
            }
        }
    }

    Component {
        id: settingsPageComponent

        SettingsPage {
            onBackRequested: pageLoader.sourceComponent = homePageComponent
            onCategoryManageRequested: pageLoader.sourceComponent = categoryManagePageComponent
        }
    }

    Component {
        id: categoryManagePageComponent

        CategoryManagePage {
            onBackRequested: pageLoader.sourceComponent = settingsPageComponent
            onSubcategoryManageRequested: function(categoryId, categoryName) {
                root.managingSubcategoryCategoryId = categoryId
                root.managingSubcategoryCategoryName = categoryName
                pageLoader.sourceComponent = subcategoryManagePageComponent
            }
        }
    }

    Component {
        id: subcategoryManagePageComponent

        SubcategoryManagePage {
            categoryId: root.managingSubcategoryCategoryId
            categoryName: root.managingSubcategoryCategoryName
            onBackRequested: pageLoader.sourceComponent = categoryManagePageComponent
        }
    }

    Component {
        id: categorySummaryPageComponent

        CategorySummaryPage {
            onBackRequested: pageLoader.sourceComponent = homePageComponent
        }
    }

    Component {
        id: weeklyBudgetPageComponent

        WeeklyBudgetPage {
            onBackRequested: pageLoader.sourceComponent = homePageComponent
        }
    }

    Component {
        id: editTransactionPageComponent

        EditTransactionPage {
            transactionId: root.editingTransactionId
            onTransactionUpdated: {
                appController.refreshTransactionList()
                pageLoader.sourceComponent = transactionListPageComponent
            }
            onTransactionDeleted: {
                appController.refreshTransactionList()
                pageLoader.sourceComponent = transactionListPageComponent
            }
        }
    }
}
