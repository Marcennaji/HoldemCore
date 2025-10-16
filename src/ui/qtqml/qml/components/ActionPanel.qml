// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: actionPanel

    color: "#1e1e1e"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Action buttons row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Fold"
                enabled: tableViewModel.validActions.includes("Fold")
                Layout.fillWidth: true

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: parent.enabled ? "#ffffff" : "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.enabled ? (parent.pressed ? "#8b0000" : (parent.hovered ? "#a52a2a" : "#b22222")) : "#333333"
                    radius: 4
                }

                onClicked: bridge.onPlayerFold()
            }

            Button {
                text: "Check"
                enabled: tableViewModel.validActions.includes("Check")
                Layout.fillWidth: true

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: parent.enabled ? "#ffffff" : "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.enabled ? (parent.pressed ? "#6b8e23" : (parent.hovered ? "#8fbc8f" : "#90ee90")) : "#333333"
                    radius: 4
                }

                onClicked: bridge.onPlayerCheck()
            }

            Button {
                text: "Call"
                enabled: tableViewModel.validActions.includes("Call")
                Layout.fillWidth: true

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: parent.enabled ? "#ffffff" : "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.enabled ? (parent.pressed ? "#005a9e" : (parent.hovered ? "#0078d4" : "#007acc")) : "#333333"
                    radius: 4
                }

                onClicked: bridge.onPlayerCall()
            }

            Button {
                text: "All-In"
                enabled: tableViewModel.validActions.includes("All-In") || tableViewModel.validActions.includes("Allin")
                Layout.fillWidth: true

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    font.bold: true
                    color: parent.enabled ? "#ffffff" : "#666666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.enabled ? (parent.pressed ? "#b8860b" : (parent.hovered ? "#daa520" : "#ffd700")) : "#333333"
                    radius: 4
                }

                onClicked: bridge.onPlayerAllIn()
            }
        }

        // Bet/Raise controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: tableViewModel.validActions.includes("Bet") || tableViewModel.validActions.includes("Raise")

            Text {
                text: tableViewModel.validActions.includes("Bet") ? "Bet:" : "Raise:"
                color: "#ffffff"
                font.pixelSize: 14
            }

            Slider {
                id: betSlider
                Layout.fillWidth: true
                from: tableViewModel.minBet
                to: tableViewModel.maxBet
                value: from
                stepSize: 10

                background: Rectangle {
                    x: betSlider.leftPadding
                    y: betSlider.topPadding + betSlider.availableHeight / 2 - height / 2
                    implicitWidth: 200
                    implicitHeight: 4
                    width: betSlider.availableWidth
                    height: implicitHeight
                    radius: 2
                    color: "#3e3e42"

                    Rectangle {
                        width: betSlider.visualPosition * parent.width
                        height: parent.height
                        color: "#ffd700"
                        radius: 2
                    }
                }

                handle: Rectangle {
                    x: betSlider.leftPadding + betSlider.visualPosition * (betSlider.availableWidth - width)
                    y: betSlider.topPadding + betSlider.availableHeight / 2 - height / 2
                    implicitWidth: 20
                    implicitHeight: 20
                    radius: 10
                    color: betSlider.pressed ? "#daa520" : "#ffd700"
                    border.color: "#b8860b"
                    border.width: 2
                }
            }

            Text {
                text: "$" + Math.round(betSlider.value)
                color: "#ffd700"
                font.pixelSize: 16
                font.bold: true
                Layout.preferredWidth: 80
            }

            Button {
                text: tableViewModel.validActions.includes("Bet") ? "Bet" : "Raise"
                Layout.preferredWidth: 100

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    color: "#ffffff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.pressed ? "#b8860b" : (parent.hovered ? "#daa520" : "#ffd700")
                    radius: 4
                }

                onClicked: {
                    let amount = Math.round(betSlider.value);
                    if (tableViewModel.validActions.includes("Bet")) {
                        bridge.onPlayerBet(amount);
                    } else {
                        bridge.onPlayerRaise(amount);
                    }
                }
            }
        }
    }
}
