// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "StartWindow.h"

#include <ui/qtwidgets/poker_ui/PokerTableWindow.h>

#include <core/engine/EngineDefs.h>
#include <core/engine/Game.h>
#include "core/services/GlobalServices.h"

#include <core/engine/model/GameData.h>
#include <core/player/deprecated/Player.h>

using namespace std;
using namespace pkt::core;

namespace pkt::ui::qtwidgets
{

StartWindow::StartWindow(const QString& appDataPath, PokerTableWindow* tableWindow, Session* session, QWidget* parent)
    : QMainWindow(parent), myAppDataPath(appDataPath), myPokerTableWindow(tableWindow), mySession(session)
{
    setupUi(this);
    // myPokerTableWindow->setStartWindow(this);
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

    myPokerTableWindow->show();

    GameData gameData;

    gameData.maxNumberOfPlayers = spinBox_quantityPlayers->value();
    gameData.startMoney = spinBox_startCash->value();
    gameData.firstSmallBlind = spinBox_smallBlind->value();

    if (radioButton_opponentsLooseAggressive->isChecked())
        gameData.tableProfile = LargeAgressiveOpponents;
    else if (radioButton_opponentsTightAggressive->isChecked())
        gameData.tableProfile = TightAgressiveOpponents;
    else
        gameData.tableProfile = RandomOpponents;

    gameData.guiSpeed = spinBox_guiSpeed->value();

    StartData startData;
    int tmpDealerPos = 0;
    startData.numberOfPlayers = gameData.maxNumberOfPlayers;

    GlobalServices::instance().randomizer().getRand(0, startData.numberOfPlayers - 1, 1, &tmpDealerPos);
    startData.startDealerPlayerId = static_cast<unsigned>(tmpDealerPos);

    // myPokerTableWindow->GameModification();

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
} // namespace pkt::ui::qtwidgets
