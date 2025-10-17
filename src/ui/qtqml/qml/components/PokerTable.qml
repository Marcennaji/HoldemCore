// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick
import QtQuick.Layouts

Item {
    id: pokerTable

    // Elliptical table in center
    Rectangle {
        id: tableEllipse
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 700)
        height: Math.min(parent.height * 0.6, 400)
        radius: height / 2
        color: "#0a5028"
        border.color: "#8b6914"
        border.width: 8

        // Community cards area
        Row {
            id: communityCards
            anchors.centerIn: parent
            spacing: 10

            Repeater {
                model: tableViewModel.boardCards

                CardWidget {
                    cardText: modelData
                    width: 60
                    height: 85
                }
            }

            // Show placeholder if no cards
            Text {
                visible: communityCards.children.length === 0
                text: "Community Cards"
                color: "#cccccc"
                font.pixelSize: 16
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // Players arranged around the table
    Repeater {
        model: tableViewModel.players

        PlayerWidget {
            required property var modelData
            required property int index

            playerData: modelData

            // Position players in an ellipse around the table
            // Swap index 0 (human) with bottom position for better visibility
            x: {
                let totalPlayers = tableViewModel.players.length;
                let adjustedIndex = index;

                // If this is the human player (index 0), put them at the bottom (index totalPlayers/2)
                // If this is the player at bottom position, put them at top
                if (index === 0) {
                    adjustedIndex = Math.floor(totalPlayers / 2);
                } else if (index === Math.floor(totalPlayers / 2)) {
                    adjustedIndex = 0;
                }

                let angle = (adjustedIndex / totalPlayers) * Math.PI * 2 - Math.PI / 2;
                let radiusX = tableEllipse.width / 2 + 100;
                return tableEllipse.x + tableEllipse.width / 2 + Math.cos(angle) * radiusX - width / 2;
            }

            y: {
                let totalPlayers = tableViewModel.players.length;
                let adjustedIndex = index;

                // If this is the human player (index 0), put them at the bottom (index totalPlayers/2)
                // If this is the player at bottom position, put them at top
                if (index === 0) {
                    adjustedIndex = Math.floor(totalPlayers / 2);
                } else if (index === Math.floor(totalPlayers / 2)) {
                    adjustedIndex = 0;
                }

                let angle = (adjustedIndex / totalPlayers) * Math.PI * 2 - Math.PI / 2;
                let radiusY = tableEllipse.height / 2 + 80;
                return tableEllipse.y + tableEllipse.height / 2 + Math.sin(angle) * radiusY - height / 2;
            }

            isActive: modelData && modelData.id === tableViewModel.currentPlayerId
            isHuman: modelData && modelData.id === tableViewModel.humanPlayerId
        }
    }
}
