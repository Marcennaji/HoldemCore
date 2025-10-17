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

    // Convert card text (e.g., "As", "Kh") to card index (0-51)
    function cardTextToIndex(text) {
        if (text.length < 2)
            return -1;

        // Parse rank (first character)
        const rankChar = text[0];
        let rankValue = 0;
        switch (rankChar) {
        case '2':
            rankValue = 0;
            break;
        case '3':
            rankValue = 1;
            break;
        case '4':
            rankValue = 2;
            break;
        case '5':
            rankValue = 3;
            break;
        case '6':
            rankValue = 4;
            break;
        case '7':
            rankValue = 5;
            break;
        case '8':
            rankValue = 6;
            break;
        case '9':
            rankValue = 7;
            break;
        case 'T':
            rankValue = 8;
            break;
        case 'J':
            rankValue = 9;
            break;
        case 'Q':
            rankValue = 10;
            break;
        case 'K':
            rankValue = 11;
            break;
        case 'A':
            rankValue = 12;
            break;
        default:
            return -1;
        }

        // Parse suit (second character)
        const suitChar = text[1];
        let suitValue = 0;
        switch (suitChar) {
        case 'c':
            suitValue = 0;
            break;  // Clubs
        case 'd':
            suitValue = 1;
            break;  // Diamonds
        case 'h':
            suitValue = 2;
            break;  // Hearts
        case 's':
            suitValue = 3;
            break;  // Spades
        default:
            return -1;
        }

        // Card index = rank * 4 + suit
        return rankValue * 4 + suitValue;
    }

    readonly property int cardIndex: cardTextToIndex(cardText)
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
