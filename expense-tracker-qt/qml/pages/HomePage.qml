import QtQuick

Item {
    Text {
        anchors.centerIn: parent
        text: appController.testMessage()
        color: "#202124"
        font.pixelSize: 32
    }
}
