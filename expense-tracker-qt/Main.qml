pragma ComponentBehavior: Bound

import QtQuick
import "qml/pages"

Window {
    id: root

    width: 480
    height: 320
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
}
