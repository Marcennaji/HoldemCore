// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "StartWindow.h"

#include <ui/qtwidgets/poker_ui/PokerTableWindow.h>

#include <core/engine/EngineDefs.h>
#include <core/engine/game/Game.h>
#include "core/services/ServiceContainer.h"

#include <core/engine/model/GameData.h>

using namespace std;
using namespace pkt::core;

namespace pkt::ui::qtwidgets
{

StartWindow::StartWindow(const QString& appDataPath, PokerTableWindow* tableWindow, Session* session, QWidget* parent)
    : QMainWindow(parent), myAppDataPath(appDataPath), myPokerTableWindow(tableWindow), mySession(session)
{
    setupUi(this);
    // myPokerTableWindow->setStartWindow(this);
    setWindowTitle(QString(tr("HoldemCore %1").arg(HoldemCore_BETA_RELEASE_STRING)));
    setWindowIcon(QIcon(myAppDataPath + "gfx/gui/misc/windowicon.png"));
    setStatusBar(nullptr);
    installEventFilter(this);

    connect(pushButtonStartGame, &QPushButton::clicked, this, &StartWindow::startNewGame);

    show();
}

StartWindow::StartWindow(const QString& appDataPath, PokerTableWindow* tableWindow, Session* session,
                         std::shared_ptr<pkt::core::ServiceContainer> services, QWidget* parent)
    : QMainWindow(parent), myAppDataPath(appDataPath), myPokerTableWindow(tableWindow), mySession(session),
      myServices(std::move(services))
{
    setupUi(this);
    // myPokerTableWindow->setStartWindow(this);
    setWindowTitle(QString(tr("HoldemCore %1").arg(HoldemCore_BETA_RELEASE_STRING)));
    setWindowIcon(QIcon(myAppDataPath + "gfx/gui/misc/windowicon.png"));
    setStatusBar(nullptr);
    installEventFilter(this);

    connect(pushButtonStartGame, &QPushButton::clicked, this, &StartWindow::startNewGame);

    show();
}

void StartWindow::ensureServicesInitialized()
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
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

    ensureServicesInitialized();
    myServices->randomizer().getRand(0, startData.numberOfPlayers - 1, 1, &tmpDealerPos);
    startData.startDealerPlayerId = static_cast<unsigned>(tmpDealerPos);

    mySession->startGame(gameData, startData);
}

bool StartWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Close)
    {
        event->ignore();

        return QMainWindow::eventFilter(obj, event);
    }
    else
    {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}
} // namespace pkt::ui::qtwidgets
