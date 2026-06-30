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
            onEditTransactionRequested: function(transactionId) {
                root.editingTransactionId = transactionId
                pageLoader.sourceComponent = editTransactionPageComponent
            }
        }
    }

    Component {
        id: editTransactionPageComponent

        EditTransactionPage {
            transactionId: root.editingTransactionId
            onTransactionUpdated: {
                transactionListModel.refresh()
                pageLoader.sourceComponent = transactionListPageComponent
            }
            onTransactionDeleted: {
                transactionListModel.refresh()
                pageLoader.sourceComponent = transactionListPageComponent
            }
        }
    }
}
