// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: window
    width: 900
    height: 600
    visible: true
    title: "HoldemCore Mobile"

    // Dark theme colors
    color: "#1e1e1e"

    StackView {
        id: stack
        anchors.fill: parent

        initialItem: SetupScreen {
            onStartGame: config => {
                stack.push(tableScreenComponent, {
                    "configuration": config
                });

                // Big blind is always 2x small blind
                var bigBlind = config.smallBlind * 2;
                appController.startGame(config.playerCount, config.smallBlind, bigBlind, config.botProfile, config.speed);
            }
        }

        Component {
            id: tableScreenComponent

            TableScreen {
                onExitToSetup: {
                    // Call C++ controller to cleanup
                    appController.exitToSetup();

                    // Defer stack navigation to avoid destroying object while signal handler is running
                    exitTimer.start();
                }
            }
        }
    }

    // Timer to defer exit navigation to avoid crash when destroying object during signal handler
    Timer {
        id: exitTimer
        interval: 0
        repeat: false
        onTriggered: stack.pop()
    }
}
