// StartWindow.qml
import QtQuick
import QtQuick.Controls

Item {
    id: startScreen
    width: 400
    height: 300

    signal startGame(int playerCount, int startChips, string profile)

    Column {
        anchors.centerIn: parent
        spacing: 12

        Text {
            text: "PokerTraining Setup"
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            spacing: 8
            Label { text: "Number of Players:" }
            SpinBox {
                id: playerCount
                from: 2
                to: 10
                value: 6
            }
        }

        Row {
            spacing: 8
            Label { text: "Starting Chips:" }
            SpinBox {
                id: chipCount
                from: 100
                to: 10000
                stepSize: 100
                value: 1000
            }
        }

        GroupBox {
            title: "Opponent Profile"
            width: parent.width
            Column {
                RadioButton {
                    id: looseAggressive
                    text: "Loose Aggressive"
                    checked: true
                }
                RadioButton {
                    id: tightAggressive
                    text: "Tight Aggressive"
                }
                RadioButton {
                    id: randomProfile
                    text: "Random"
                }
            }
        }

        Button {
            text: "Start Game"
            onClicked: {
                let profile = looseAggressive.checked ? "LARGE_AGRESSIVE_OPPONENTS"
                             : tightAggressive.checked ? "TIGHT_AGRESSIVE_OPPONENTS"
                             : "RANDOM_OPPONENTS";
                startGame(playerCount.value, chipCount.value, profile);
            }
        }
    }
}
