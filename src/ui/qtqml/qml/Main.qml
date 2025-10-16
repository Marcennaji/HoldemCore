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
                // Call C++ controller to start game
                appController.startGame(config.playerCount, config.smallBlind, config.bigBlind, config.botProfile, config.speed);

                // Navigate to table screen
                stack.push(tableScreenComponent, {
                    "configuration": config
                });
            }
        }

        Component {
            id: tableScreenComponent

            TableScreen {
                onExitToSetup: {
                    // Call C++ controller to cleanup
                    appController.exitToSetup();

                    // Navigate back to setup
                    stack.pop();
                }
            }
        }
    }
}
