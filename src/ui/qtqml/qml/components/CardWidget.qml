// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick

Rectangle {
    id: card

    property string cardText: ""

    width: 60
    height: 85
    radius: 6
    color: "#ffffff"
    border.color: "#333333"
    border.width: 1

    // Determine if card is hidden
    readonly property bool isHidden: cardText === "" || cardText === "??"

    // Determine card color based on suit
    readonly property color cardColor: {
        if (isHidden)
            return "#333333";
        if (cardText.includes("♥") || cardText.includes("♦"))
            return "#dc143c";
        return "#000000";
    }

    // Card back pattern
    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: 4
        visible: isHidden

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#1e3a8a"
            }
            GradientStop {
                position: 0.5
                color: "#3b82f6"
            }
            GradientStop {
                position: 1.0
                color: "#1e3a8a"
            }
        }

        // Pattern
        Grid {
            anchors.centerIn: parent
            columns: 3
            rows: 5
            spacing: 2

            Repeater {
                model: 15
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: "#60a5fa"
                    opacity: 0.5
                }
            }
        }
    }

    // Card face
    Text {
        anchors.centerIn: parent
        text: cardText
        font.pixelSize: Math.min(card.width * 0.5, 28)
        font.bold: true
        color: cardColor
        visible: !isHidden
    }

    // Small rank/suit in corners (optional, for realistic look)
    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 4
        text: cardText
        font.pixelSize: 10
        font.bold: true
        color: cardColor
        visible: !isHidden
    }
}
