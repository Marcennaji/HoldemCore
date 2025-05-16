import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: gameTable
    title: "PokerTraining – Game Table"
    width: 800
    height: 600
    visible: true

    signal fold()
    signal call()
    signal raise()

    Rectangle {
        id: table
        anchors.fill: parent
        color: "#084B4F"
        radius: 20
        border.color: "#004D40"
        border.width: 4

        // Placeholder for pot
        Label {
            id: potLabel
            text: "Pot: $450"
            font.pixelSize: 18
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 20
            color: "white"
        }

        // Placeholder for board cards
        Row {
            id: board
            spacing: 8
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: potLabel.bottom
            anchors.topMargin: 16

            Repeater {
                model: 5
                delegate: Rectangle {
                    width: 60
                    height: 90
                    radius: 4
                    color: "white"
                    border.color: "#444"
                    Text {
                        anchors.centerIn: parent
                        text: "?"
                        font.pixelSize: 24
                    }
                }
            }
        }

        // Player action buttons
        Row {
            spacing: 16
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 24

            Button {
                text: "Fold"
                onClicked: gameTable.fold()
            }
            Button {
                text: "Call"
                onClicked: gameTable.call()
            }
            Button {
                text: "Raise"
                onClicked: gameTable.raise()
            }
        }
    }
}
