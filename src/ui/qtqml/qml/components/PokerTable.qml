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

            // Calculate angle for this player's position
            // Position players directly by index (same as Qt Widgets version)
            // Player at index 0 is at bottom (π/2), others arranged clockwise
            property real playerAngle: {
                let totalPlayers = tableViewModel.players.length;
                let angleStep = (Math.PI * 2) / totalPlayers;
                let startAngle = Math.PI / 2;  // Bottom position (6 o'clock)
                return startAngle + (index * angleStep);
            }

            // Position players in an ellipse around the table
            x: {
                let radiusX = tableEllipse.width / 2 + 100;
                return tableEllipse.x + tableEllipse.width / 2 + Math.cos(playerAngle) * radiusX - width / 2;
            }

            y: {
                let radiusY = tableEllipse.height / 2 + 80;
                return tableEllipse.y + tableEllipse.height / 2 + Math.sin(playerAngle) * radiusY - height / 2;
            }

            // Determine dealer button position based on player's angle
            // The button should point toward the table center
            dealerButtonPosition: {
                let angle = playerAngle;
                // Normalize angle to 0-2π
                while (angle < 0)
                    angle += Math.PI * 2;
                while (angle >= Math.PI * 2)
                    angle -= Math.PI * 2;

                // Determine position based on angle quadrant
                // Top: -π/2 ± π/4 (around -90°)
                // Right: 0 ± π/4 (around 0°/360°)
                // Bottom: π/2 ± π/4 (around 90°)
                // Left: π ± π/4 (around 180°)

                if (angle > Math.PI * 7 / 4 || angle <= Math.PI / 4) {
                    return "left";  // Player on right side of table
                } else if (angle > Math.PI / 4 && angle <= Math.PI * 3 / 4) {
                    return "top";   // Player on bottom side of table
                } else if (angle > Math.PI * 3 / 4 && angle <= Math.PI * 5 / 4) {
                    return "right"; // Player on left side of table
                } else {
                    return "bottom"; // Player on top side of table
                }
            }

            isActive: modelData && modelData.id === tableViewModel.currentPlayerId
            isHuman: modelData && modelData.id === tableViewModel.humanPlayerId
        }
    }
}
