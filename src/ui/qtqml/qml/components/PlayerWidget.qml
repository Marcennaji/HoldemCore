// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: playerWidget

    property var playerData
    property bool isActive: false
    property bool isHuman: false
    property bool isFolded: playerData && playerData.folded ? playerData.folded : false

    width: 140
    height: 145
    color: isActive ? "#3e3e42" : "#2d2d30"
    border.color: isHuman ? "#ffd700" : (isActive ? "#007acc" : "#555555")
    border.width: isActive ? 3 : 2
    radius: 8
    opacity: isFolded ? 0.5 : 1.0

    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // Player name and chips on same line
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Text {
                Layout.fillWidth: true
                text: playerData ? playerData.name : ""
                color: "#ffffff"
                font.pixelSize: 14
                font.bold: isHuman
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideRight
            }

            Text {
                text: playerData ? "$" + playerData.chips : ""
                color: "#4ec9b0"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignRight
            }
        }

        // Hole cards
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 4
            visible: playerData && playerData.hasCards
            opacity: isFolded ? 0.35 : 1.0

            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }

            CardWidget {
                cardText: playerData && playerData.card1 ? playerData.card1 : ""
                width: 35
                height: 50
            }

            CardWidget {
                cardText: playerData && playerData.card2 ? playerData.card2 : ""
                width: 35
                height: 50
            }
        }

        // Last action
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 25
            color: "#1e1e1e"
            radius: 4
            visible: playerData && playerData.lastAction !== ""

            Text {
                anchors.centerIn: parent
                text: {
                    if (!playerData || !playerData.lastAction)
                        return "";
                    let action = playerData.lastAction;
                    if (playerData.lastBet > 0) {
                        return action + " $" + playerData.lastBet;
                    }
                    return action;
                }
                color: "#ffffff"
                font.pixelSize: 11
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }

    // Active indicator pulse animation
    SequentialAnimation on border.width {
        running: isActive
        loops: Animation.Infinite

        NumberAnimation {
            to: 4
            duration: 500
        }
        NumberAnimation {
            to: 3
            duration: 500
        }
    }
}
