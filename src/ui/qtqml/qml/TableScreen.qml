// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

Page {
    id: tableScreen

    property var configuration
    signal exitToSetup

    background: Rectangle {
        color: "#0d6e3a"  // Poker table green
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header with game info
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#1e1e1e"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 20

                Text {
                    text: tableViewModel.gameStateText
                    color: "#ffffff"
                    font.pixelSize: 18
                    font.bold: true
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: "Pot: $" + tableViewModel.potAmount
                    color: "#ffd700"
                    font.pixelSize: 18
                    font.bold: true
                }

                Button {
                    text: "Exit"
                    onClicked: tableScreen.exitToSetup()

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.pressed ? "#8b0000" : (parent.hovered ? "#a52a2a" : "#b22222")
                        radius: 4
                        implicitWidth: 80
                        implicitHeight: 35
                    }
                }
            }
        }

        // Main poker table area
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            PokerTable {
                id: pokerTable
                anchors.fill: parent
                anchors.margins: 20
            }
        }

        // Action panel for human player
        ActionPanel {
            id: actionPanel
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            visible: tableViewModel.awaitingHumanInput
        }

        // Hand result display
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "#1e1e1e"
            visible: tableViewModel.handResult !== ""

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    text: tableViewModel.handResult
                    color: "#ffd700"
                    font.pixelSize: 16
                    font.bold: true
                    Layout.fillWidth: true
                }

                Button {
                    text: "Next Hand"

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
                        implicitWidth: 120
                        implicitHeight: 35
                    }

                    onClicked: {
                        bridge.onNextHandRequested();
                    }
                }
            }
        }
    }
}
