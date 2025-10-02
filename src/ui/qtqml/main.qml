import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: mainWindow
    width: 1024
    height: 768
    visible: true
    title: qsTr("HoldemCore - QML Edition")

    color: '#97ada7' 

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                text: qsTr("HoldemCore")
                elide: Label.ElideRight
                Layout.fillWidth: true
                font.pixelSize: 18
                font.bold: true
                color: "white"
            }
            ToolButton {
                text: qsTr("Settings")
                onClicked: {
                    // TODO: Open settings dialog
                }
            }
        }
    }

    // Main content area
    Rectangle {
        anchors.fill: parent
        color: "transparent"

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 20

            Label {
                text: qsTr("Welcome to HoldemCore")
                font.pixelSize: 32
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: qsTr("QML Interface")
                font.pixelSize: 16
                color: "#CCCCCC"
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: qsTr("Start New Game")
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 40
                
                onClicked: {
                    // TODO: Implement game start logic
                    console.log("Starting new game...")
                }
            }

            Button {
                text: qsTr("Load Game")
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 40
                
                onClicked: {
                    // TODO: Implement load game logic
                    console.log("Loading game...")
                }
            }

            Button {
                text: qsTr("Statistics")
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 40
                
                onClicked: {
                    // TODO: Show statistics
                    console.log("Showing statistics...")
                }
            }
        }
    }

    // Status bar
    footer: ToolBar {
        Label {
            text: qsTr("Ready")
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
        }
    }
}