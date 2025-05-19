/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#include "StartWindow.h"

#include <ui/qtwidgets/GuiWrapper.h>
#include <ui/qtwidgets/gametable/GameTableWindow.h>
#include <ui/qtwidgets/gametable/GuiDisplayGameActions.h>

#include <core/engine/EngineDefs.h>
#include <core/engine/Game.h>
#include <core/engine/Randomizer.h>
#include <core/engine/model/Gamedata.h>
#include <core/player/Player.h>

using namespace std;

StartWindow::StartWindow(const QString& appDataPath, IGui* gui, Session* session, QWidget* parent)
    : QMainWindow(parent), myAppDataPath(appDataPath), myGuiInterface(gui), mySession(session)
{
    setupUi(this);
    myGuiInterface->setStartWindow(this);
    setWindowTitle(QString(tr("PokerTraining %1").arg(POKERTRAINING_BETA_RELEASE_STRING)));
    setWindowIcon(QIcon(myAppDataPath + "gfx/gui/misc/windowicon.png"));
    setStatusBar(nullptr);
    installEventFilter(this);

    connect(pushButtonStartGame, &QPushButton::clicked, this, &StartWindow::startNewGame);

    show();
}
StartWindow::~StartWindow()
{
}

void StartWindow::startNewGame()
{

    this->hide();
    auto gameTableWindow = static_cast<GameTableWindow*>(myGuiInterface->getGameTableWindow());
    gameTableWindow->show();

    GameData gameData;

    gameData.maxNumberOfPlayers = spinBox_quantityPlayers->value();
    gameData.startMoney = spinBox_startCash->value();
    gameData.firstSmallBlind = spinBox_smallBlind->value();

    if (radioButton_opponentsLooseAggressive->isChecked())
        gameData.tableProfile = LARGE_AGRESSIVE_OPPONENTS;
    else if (radioButton_opponentsTightAgressive->isChecked())
        gameData.tableProfile = TIGHT_AGRESSIVE_OPPONENTS;
    else
        gameData.tableProfile = RANDOM_OPPONENTS;

    gameData.guiSpeed = GAME_SPEED;

    StartData startData;
    int tmpDealerPos = 0;
    startData.numberOfPlayers = gameData.maxNumberOfPlayers;

    Randomizer::GetRand(0, startData.numberOfPlayers - 1, 1, &tmpDealerPos);
    startData.startDealerPlayerId = static_cast<unsigned>(tmpDealerPos);

    gameTableWindow->GameModification();

    mySession->startGame(gameData, startData);
}

bool StartWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Close)
    {
        event->ignore();
        //        mySession->getLog()->closeLogDbAtExit();
        return QMainWindow::eventFilter(obj, event);
    }
    else
    {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}
