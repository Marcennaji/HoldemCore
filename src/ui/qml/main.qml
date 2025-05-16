import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "PokerTraining (QML)"

    Column {
        anchors.centerIn: parent
        spacing: 20

        Label {
            text: "Welcome to QML PokerTraining!"
        }

        Button {
            text: "Start Game"
            onClicked: console.log("Start game clicked")
        }
    }
}
