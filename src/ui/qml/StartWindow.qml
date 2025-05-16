import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: startWindow
    title: "PokerTraining – Setup Game"
    width: 400
    height: 300
    visible: true

    signal startGame(int players, int chips, string profile)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Label {
            text: "Number of Players"
        }
        SpinBox {
            id: playerCount
            from: 2
            to: 9
            value: 6
        }

        Label {
            text: "Starting Chips"
        }
        SpinBox {
            id: chipCount
            from: 100
            to: 10000
            stepSize: 100
            value: 1500
        }

        GroupBox {
            title: "Bot Opponent Profile"
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 8

                RadioButton { id: tight; text: "Tight Aggressive"; checked: true }
                RadioButton { id: loose; text: "Loose Aggressive" }
                RadioButton { id: random; text: "Random Mix" }
            }
        }

        Button {
            text: "Start Game"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                var profile = tight.checked ? "TIGHT_AGGRESSIVE_OPPONENTS"
                              : loose.checked ? "LARGE_AGRESSIVE_OPPONENTS"
                              : "RANDOM_OPPONENTS";
                startGame(playerCount.value, chipCount.value, profile)
            }
        }
    }
}
