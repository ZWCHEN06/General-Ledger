pragma ComponentBehavior: Bound

import QtQuick
import "qml/pages"

Window {
    id: root

    property int editingTransactionId: 0

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
            onSettingsRequested: pageLoader.sourceComponent = settingsPageComponent
        }
    }

    Component {
        id: addTransactionPageComponent

        AddTransactionPage {
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
        }
    }

    Component {
        id: categorySummaryPageComponent

        CategorySummaryPage {
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
