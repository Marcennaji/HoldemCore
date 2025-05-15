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
#include "startwindowimpl.h"

#include <ui/qtwidgets/gametable/GuiDisplayGameActions.h>
#include <ui/qtwidgets/gametable/gametableimpl.h>
#include <ui/qtwidgets/guiwrapper.h>
#include <ui/qtwidgets/newgamedialog/newgamedialogimpl.h>

#include <core/engine/EngineDefs.h>
#include <core/engine/Randomizer.h>
#include <core/engine/game.h>
#include <core/engine/model/Gamedata.h>
#include <core/player/Player.h>

using namespace std;

startWindowImpl::startWindowImpl(const QString& appDataPath, IGui* gui, Session* session, QWidget* parent)
    : QMainWindow(parent), myAppDataPath(appDataPath), myGuiInterface(gui), mySession(session)
{
    setupUi(this);
    myGuiInterface->setStartWindow(this);
    setWindowTitle(QString(tr("PokerTraining %1").arg(POKERTRAINING_BETA_RELEASE_STRING)));
    setWindowIcon(QIcon(myAppDataPath + "gfx/gui/misc/windowicon.png"));
    setStatusBar(nullptr);
    installEventFilter(this);

    myNewGameDialog = new newGameDialogImpl(this);
    connect(actionStartGame, &QAction::triggered, this, &startWindowImpl::callNewGameDialog);
    connect(pushButtonStartGame, &QPushButton::clicked, this, &startWindowImpl::callNewGameDialog);

    show();
}
startWindowImpl::~startWindowImpl()
{
}

void startWindowImpl::callNewGameDialog()
{
    myNewGameDialog->exec();
    if (myNewGameDialog->result() == QDialog::Accepted)
    {
        startNewGame(myNewGameDialog);
    }
}

void startWindowImpl::startNewGame(newGameDialogImpl* v)
{

    this->hide();
    myGuiInterface->getW()->show();

    // get values from  game dialog
    GameData gameData;
    if (v)
    {
        // Set Game Data
        gameData.maxNumberOfPlayers = v->spinBox_quantityPlayers->value();
        gameData.startMoney = v->spinBox_startCash->value();
        gameData.firstSmallBlind = GAME_START_SBLIND;

        // Speeds
        gameData.guiSpeed = 8;

        if (v->radioButton_opponentsLooseAggressive->isChecked())
            gameData.tableProfile = LARGE_AGRESSIVE_OPPONENTS;
        else if (v->radioButton_opponentsTightAgressive->isChecked())
            gameData.tableProfile = TIGHT_AGRESSIVE_OPPONENTS;
        else
            gameData.tableProfile = RANDOM_OPPONENTS;
    }
    // start with default values
    else
    {
        // Set Game Data
        gameData.maxNumberOfPlayers = GAME_NUMBER_OF_PLAYERS;
        gameData.startMoney = GAME_START_CASH;
        gameData.firstSmallBlind = GAME_START_SBLIND;
        gameData.tableProfile = TIGHT_AGRESSIVE_OPPONENTS;

        // Speeds
        gameData.guiSpeed = GAME_SPEED;
    }
    // Set dealer pos.
    StartData startData;
    int tmpDealerPos = 0;
    startData.numberOfPlayers = gameData.maxNumberOfPlayers;

    Randomizer::GetRand(0, startData.numberOfPlayers - 1, 1, &tmpDealerPos);
    // if(DEBUG_MODE) {
    //     tmpDealerPos = 4;
    // }
    startData.startDealerPlayerId = static_cast<unsigned>(tmpDealerPos);

    // some gui modifications
    myGuiInterface->getW()->GameModification();

    mySession->startGame(gameData, startData);
}

bool startWindowImpl::eventFilter(QObject* obj, QEvent* event)
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
