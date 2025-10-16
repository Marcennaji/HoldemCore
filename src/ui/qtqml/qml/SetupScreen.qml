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
                to: 9
                value: 6
                editable: true

                contentItem: TextInput {
                    text: playerCount.displayText
                    font: playerCount.font
                    color: "#ffffff"
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    readOnly: !playerCount.editable
                }

                background: Rectangle {
                    color: "#2d2d30"
                    border.color: "#3e3e42"
                    border.width: 1
                    radius: 4
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
                from: 5
                to: 500
                value: 25
                stepSize: 5
                editable: true

                contentItem: TextInput {
                    text: smallBlind.displayText
                    font: smallBlind.font
                    color: "#ffffff"
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    readOnly: !smallBlind.editable
                }

                background: Rectangle {
                    color: "#2d2d30"
                    border.color: "#3e3e42"
                    border.width: 1
                    radius: 4
                }
            }
        }

        // Big blind
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Big Blind:"
                color: "#cccccc"
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            SpinBox {
                id: bigBlind
                from: 10
                to: 1000
                value: 50
                stepSize: 10
                editable: true

                contentItem: TextInput {
                    text: bigBlind.displayText
                    font: bigBlind.font
                    color: "#ffffff"
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    readOnly: !bigBlind.editable
                }

                background: Rectangle {
                    color: "#2d2d30"
                    border.color: "#3e3e42"
                    border.width: 1
                    radius: 4
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
                    font: botProfile.font
                    color: "#ffffff"
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: "#2d2d30"
                    border.color: "#3e3e42"
                    border.width: 1
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
                    smallBlind: smallBlind.value,
                    bigBlind: bigBlind.value,
                    botProfile: botProfile.currentText,
                    speed: speedSlider.value
                });
            }
        }
    }
}
