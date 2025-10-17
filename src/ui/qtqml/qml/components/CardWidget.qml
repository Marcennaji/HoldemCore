// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick
import HoldemCore 1.0

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

    // Use engine's CardHelper for guaranteed index consistency
    // This calls the C++ CardUtilities::getCardIndex() directly
    readonly property int cardIndex: isHidden ? -1 : CardHelper.getCardIndex(cardText)
    readonly property bool useImages: cardIndex >= 0

    // Determine card color based on suit
    readonly property color cardColor: {
        if (isHidden)
            return "#333333";
        if (cardText.includes("♥") || cardText.includes("♦"))
            return "#dc143c";
        return "#000000";
    }

    // Card image (try resource first, fallback to filesystem)
    Image {
        id: cardImage
        anchors.fill: parent
        anchors.margins: 1
        source: {
            if (!useImages && !isHidden)
                return "";

            // Try resource path first
            let resourcePath = isHidden ? ":/cards/flipside.png" : ":/cards/" + cardIndex + ".png";

            // Fallback to filesystem path (WORKAROUND for resource loading issue)
            let filesystemPath = isHidden ? cardsImagePath + "/flipside.png" : cardsImagePath + "/" + cardIndex + ".png";

            // Use filesystem path since resources aren't loading
            return filesystemPath;
        }
        fillMode: Image.PreserveAspectFit
        smooth: true
        visible: (useImages || isHidden) && status === Image.Ready

        onStatusChanged: {
            if (status === Image.Error) {
                console.warn("Failed to load card image:", source, "for card:", cardText);
            }
        }
    }

    // Fallback: Card back pattern (if image not available)
    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: 4
        visible: isHidden && !useImages

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

    // Fallback: Card face text (always show if image failed or not using images)
    Text {
        anchors.centerIn: parent
        text: cardText
        font.pixelSize: Math.min(card.width * 0.5, 28)
        font.bold: true
        color: cardColor
        visible: !isHidden && (cardImage.status !== Image.Ready || !useImages)
    }
}
