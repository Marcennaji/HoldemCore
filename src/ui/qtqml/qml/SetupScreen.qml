// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: setupScreen

    signal startGame(var configuration)

    background: Rectangle {
        color: "#1e1e1e"
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: Math.min(parent.width * 0.8, 400)

        // Title
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "HoldemCore Setup"
            font.pixelSize: 32
            font.bold: true
            color: "#ffffff"
        }

        // Player count
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Number of Players:"
                color: "#cccccc"
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            SpinBox {
                id: playerCount
                from: 2
                to: 10
                value: 6
                editable: false
                Layout.preferredWidth: 120

                // Hide default up/down indicators by setting width to 0
                up.indicator.width: 0
                down.indicator.width: 0

                contentItem: Rectangle {
                    color: "#1e1e1e"
                    border.color: "#007acc"
                    border.width: 2
                    radius: 4

                    // Minus button
                    Rectangle {
                        id: playerCountMinus
                        anchors.left: parent.left
                        width: 40
                        height: parent.height
                        color: playerCountMinusArea.pressed ? "#3e3e42" : "#2d2d30"
                        border.color: "#007acc"
                        radius: 4

                        Text {
                            text: "-"
                            font.pixelSize: 20
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: playerCountMinusArea
                            anchors.fill: parent
                            onClicked: {
                                if (playerCount.value > playerCount.from) {
                                    playerCount.value--;
                                }
                            }
                        }
                    }

                    // Value display
                    Text {
                        anchors.centerIn: parent
                        text: playerCount.value
                        font.pixelSize: 18
                        color: "#ffffff"
                    }

                    // Plus button
                    Rectangle {
                        id: playerCountPlus
                        anchors.right: parent.right
                        width: 40
                        height: parent.height
                        color: playerCountPlusArea.pressed ? "#3e3e42" : "#2d2d30"
                        border.color: "#007acc"
                        radius: 4

                        Text {
                            text: "+"
                            font.pixelSize: 20
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: playerCountPlusArea
                            anchors.fill: parent
                            onClicked: {
                                if (playerCount.value < playerCount.to) {
                                    playerCount++;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Start cash
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Start Cash:"
                color: "#cccccc"
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            SpinBox {
                id: startCash
                from: 1000
                to: 1000000
                value: 2000
                stepSize: 50
                editable: false
                Layout.preferredWidth: 120

                property string prefix: "$"

                // Hide default up/down indicators
                up.indicator.width: 0
                down.indicator.width: 0

                contentItem: Rectangle {
                    color: "#1e1e1e"
                    border.color: "#007acc"
                    border.width: 2
                    radius: 4

                    // Minus button
                    Rectangle {
                        anchors.left: parent.left
                        width: 40
                        height: parent.height
                        color: startCashMinusArea.pressed ? "#3e3e42" : "#2d2d30"
                        border.color: "#007acc"
                        radius: 4

                        Text {
                            text: "-"
                            font.pixelSize: 20
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: startCashMinusArea
                            anchors.fill: parent
                            onClicked: {
                                if (startCash.value > startCash.from) {
                                    startCash.value -= startCash.stepSize;
                                }
                            }
                        }
                    }

                    // Value display
                    Text {
                        anchors.centerIn: parent
                        text: startCash.prefix + startCash.value
                        font.pixelSize: 18
                        color: "#ffffff"
                    }

                    // Plus button
                    Rectangle {
                        anchors.right: parent.right
                        width: 40
                        height: parent.height
                        color: startCashPlusArea.pressed ? "#3e3e42" : "#2d2d30"
                        border.color: "#007acc"
                        radius: 4

                        Text {
                            text: "+"
                            font.pixelSize: 20
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: startCashPlusArea
                            anchors.fill: parent
                            onClicked: {
                                if (startCash.value < startCash.to) {
                                    startCash.value += startCash.stepSize;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Small blind
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Small Blind:"
                color: "#cccccc"
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            SpinBox {
                id: smallBlind
                from: 1
                to: 1000
                value: 10
                stepSize: 5
                editable: false
                Layout.preferredWidth: 120

                property string prefix: "$"

                // Hide default up/down indicators
                up.indicator.width: 0
                down.indicator.width: 0

                contentItem: Rectangle {
                    color: "#1e1e1e"
                    border.color: "#007acc"
                    border.width: 2
                    radius: 4

                    // Minus button
                    Rectangle {
                        anchors.left: parent.left
                        width: 40
                        height: parent.height
                        color: smallBlindMinusArea.pressed ? "#3e3e42" : "#2d2d30"
                        border.color: "#007acc"
                        radius: 4

                        Text {
                            text: "-"
                            font.pixelSize: 20
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: smallBlindMinusArea
                            anchors.fill: parent
                            onClicked: {
                                if (smallBlind.value > smallBlind.from) {
                                    smallBlind.value -= smallBlind.stepSize;
                                }
                            }
                        }
                    }

                    // Value display
                    Text {
                        anchors.centerIn: parent
                        text: smallBlind.prefix + smallBlind.value
                        font.pixelSize: 18
                        color: "#ffffff"
                    }

                    // Plus button
                    Rectangle {
                        anchors.right: parent.right
                        width: 40
                        height: parent.height
                        color: smallBlindPlusArea.pressed ? "#3e3e42" : "#2d2d30"
                        border.color: "#007acc"
                        radius: 4

                        Text {
                            text: "+"
                            font.pixelSize: 20
                            color: "#ffffff"
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: smallBlindPlusArea
                            anchors.fill: parent
                            onClicked: {
                                if (smallBlind.value < smallBlind.to) {
                                    smallBlind.value += smallBlind.stepSize;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Bot profile
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Bot Profile:"
                color: "#cccccc"
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            ComboBox {
                id: botProfile
                model: ["Tight", "Loose", "Aggressive"]
                currentIndex: 0

                contentItem: Text {
                    leftPadding: 10
                    text: botProfile.displayText
                    font.pixelSize: 18
                    color: "#ffffff"
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: "#3d3d40"
                    border.color: "#007acc"
                    border.width: 2
                    radius: 4
                }

                popup: Popup {
                    y: botProfile.height
                    width: botProfile.width
                    padding: 1

                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: botProfile.popup.visible ? botProfile.delegateModel : null
                        currentIndex: botProfile.highlightedIndex
                    }

                    background: Rectangle {
                        color: "#2d2d30"
                        border.color: "#3e3e42"
                        border.width: 1
                    }
                }

                delegate: ItemDelegate {
                    width: botProfile.width
                    text: modelData

                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font: botProfile.font
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.highlighted ? "#3e3e42" : "#2d2d30"
                    }
                }
            }
        }

        // Game speed
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Game Speed:"
                color: "#cccccc"
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            Slider {
                id: speedSlider
                from: 0.5
                to: 3.0
                value: 1.0
                stepSize: 0.5

                background: Rectangle {
                    x: speedSlider.leftPadding
                    y: speedSlider.topPadding + speedSlider.availableHeight / 2 - height / 2
                    implicitWidth: 200
                    implicitHeight: 4
                    width: speedSlider.availableWidth
                    height: implicitHeight
                    radius: 2
                    color: "#3e3e42"

                    Rectangle {
                        width: speedSlider.visualPosition * parent.width
                        height: parent.height
                        color: "#007acc"
                        radius: 2
                    }
                }

                handle: Rectangle {
                    x: speedSlider.leftPadding + speedSlider.visualPosition * (speedSlider.availableWidth - width)
                    y: speedSlider.topPadding + speedSlider.availableHeight / 2 - height / 2
                    implicitWidth: 16
                    implicitHeight: 16
                    radius: 8
                    color: speedSlider.pressed ? "#005a9e" : "#007acc"
                }
            }

            Text {
                text: speedSlider.value.toFixed(1) + "x"
                color: "#ffffff"
                font.pixelSize: 14
                Layout.preferredWidth: 40
            }
        }

        // Start button
        Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20
            text: "Start Game"
            font.pixelSize: 18
            font.bold: true

            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: parent.pressed ? "#005a9e" : (parent.hovered ? "#0078d4" : "#007acc")
                radius: 4
                implicitWidth: 150
                implicitHeight: 50
            }

            onClicked: {
                setupScreen.startGame({
                    playerCount: playerCount.value,
                    startCash: startCash.value,
                    smallBlind: smallBlind.value,
                    botProfile: botProfile.currentText,
                    speed: speedSlider.value
                });
            }
        }
    }
}
