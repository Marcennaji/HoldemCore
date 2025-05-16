// GameTable.qml
import QtQuick
import QtQuick.Controls

Item {
    id: gameTable
    width: 800
    height: 600

    signal backToStart()

    Rectangle {
        anchors.fill: parent
        color: "#0e4d25"

        Text {
            text: "Game Table View"
            color: "white"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 24
            padding: 20
        }

        Button {
            text: "Back to Start"
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 20
            onClicked: gameTable.backToStart()
        }
    }
}
