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
    property bool isDealer: playerData && playerData.isDealer ? playerData.isDealer : false
    property string dealerButtonPosition: "bottom"  // "top", "bottom", "left", or "right"

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

    // Dealer button indicator (positioned based on player's location around table)
    Rectangle {
        visible: isDealer
        width: 32
        height: 32
        radius: 16
        color: "#ffffff"
        border.color: "#000000"
        border.width: 2
        z: 10  // Ensure it's above other elements

        // Position based on dealerButtonPosition property
        anchors.horizontalCenter: {
            if (dealerButtonPosition === "left" || dealerButtonPosition === "right") {
                return undefined;
            }
            return parent.horizontalCenter;
        }
        anchors.verticalCenter: {
            if (dealerButtonPosition === "top" || dealerButtonPosition === "bottom") {
                return undefined;
            }
            return parent.verticalCenter;
        }

        // Horizontal positioning
        // "left" means button should be on the left side (player is on right side of table)
        anchors.left: dealerButtonPosition === "left" ? parent.left : undefined
        anchors.leftMargin: dealerButtonPosition === "left" ? -36 : 0  // Button width (32) + margin (4)
        // "right" means button should be on the right side (player is on left side of table)
        anchors.right: dealerButtonPosition === "right" ? parent.right : undefined
        anchors.rightMargin: dealerButtonPosition === "right" ? -36 : 0

        // Vertical positioning
        // "top" means button should be on top (player is on bottom side of table)
        anchors.top: dealerButtonPosition === "top" ? parent.top : undefined
        anchors.topMargin: dealerButtonPosition === "top" ? -36 : 0
        // "bottom" means button should be on bottom (player is on top side of table)
        anchors.bottom: dealerButtonPosition === "bottom" ? parent.bottom : undefined
        anchors.bottomMargin: dealerButtonPosition === "bottom" ? -36 : 0

        Text {
            anchors.centerIn: parent
            text: "D"
            color: "#000000"
            font.pixelSize: 16
            font.bold: true
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
