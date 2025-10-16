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
            x: {
                let angle = (index / tableViewModel.players.length) * Math.PI * 2 - Math.PI / 2;
                let radiusX = tableEllipse.width / 2 + 80;
                return tableEllipse.x + tableEllipse.width / 2 + Math.cos(angle) * radiusX - width / 2;
            }

            y: {
                let angle = (index / tableViewModel.players.length) * Math.PI * 2 - Math.PI / 2;
                let radiusY = tableEllipse.height / 2 + 60;
                return tableEllipse.y + tableEllipse.height / 2 + Math.sin(angle) * radiusY - height / 2;
            }

            isActive: modelData && modelData.id === tableViewModel.currentPlayerId
            isHuman: modelData && modelData.id === tableViewModel.humanPlayerId
        }
    }
}
