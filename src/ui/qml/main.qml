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
        id: gameTable
        GameTable {
            onBackToStart: nav.pop()
        }
    }

    Component.onCompleted: backend.gameStarted.connect(() => {
        nav.push(gameTable)
    });    
}
