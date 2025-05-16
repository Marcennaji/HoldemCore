import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "PokerTraining (QML)"

    StackView {
        id: nav
        anchors.fill: parent
        initialItem: startWindow
    }

    Component {
        id: startWindow
        StartWindow {
            onStartGame: (players, chips, profile) => {
                console.log("StartGame called with:", players, chips, profile)
                nav.push(gameTable)
            }
        }
    }

    Component {
        id: gameTable
        GameTable {
            onBackToStart: nav.pop()
        }
    }
}
