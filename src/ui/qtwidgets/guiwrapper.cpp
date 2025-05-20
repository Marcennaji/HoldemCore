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
#include "guiwrapper.h"
#include <core/engine/Game.h>
#include <core/player/Player.h>
#include <core/session/Session.h>
#include <infra/persistence/SqliteLogStore.h>
#include <ui/qtwidgets/gametable/GameTableWindow.h>
#include <ui/qtwidgets/gametable/GuiDisplayGameActions.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

using namespace std;

GuiWrapper::GuiWrapper(const std::string& appDataDir, StartWindow* s)
    : myGuiDisplayGameActions(NULL), myW(NULL), myStartWindow(s)
{

    myW = new GameTableWindow(appDataDir);
    myGuiDisplayGameActions = new GuiDisplayGameActions(myW);
}

GuiWrapper::~GuiWrapper()
{
    delete myGuiDisplayGameActions;
}
void GuiWrapper::setStartWindow(void* w)
{
    myStartWindow = static_cast<StartWindow*>(w);
    if (myGuiDisplayGameActions)
        myStartWindow->setGuiDisplayGameActions(myGuiDisplayGameActions);
    if (myW)
        myW->setStartWindow(myStartWindow);
}

void GuiWrapper::initGui(int speed)
{
    myW->signalInitGui(speed);
}

Session* GuiWrapper::getSession()
{
    return myStartWindow->getSession();
}
void GuiWrapper::setSession(Session* /*session*/)
{
    // myStartWindow->setSession(session);
}

void GuiWrapper::refreshAction(int playerID, int playerAction) const
{
    myW->signalRefreshAction(playerID, playerAction);
}
void GuiWrapper::refreshAll() const
{
    myW->signalRefreshAll();
}
void GuiWrapper::refreshGroupbox(int playerID, int status) const
{
    myW->signalRefreshGroupbox(playerID, status);
}

void GuiWrapper::refreshButton() const
{
    myW->signalRefreshButton();
}
void GuiWrapper::refreshGameLabels(GameState state) const
{
    myW->signalRefreshGameLabels(state);
}

void GuiWrapper::waitForGuiUpdateDone() const
{
    myW->signalGuiUpdateDone();
    myW->waitForGuiUpdateDone();
}

void GuiWrapper::dealBettingRoundCards(int myBettingRoundID)
{
    myW->signalDealBettingRoundCards(myBettingRoundID);
}

void GuiWrapper::dealHoleCards()
{
    myW->signalDealHoleCards();
}
void GuiWrapper::dealFlopCards()
{
    myW->signalDealFlopCards0();
}
void GuiWrapper::dealTurnCard()
{
    myW->signalDealTurnCards0();
}
void GuiWrapper::dealRiverCard()
{
    myW->signalDealRiverCards0();
}

void GuiWrapper::nextPlayerAnimation()
{
    myW->signalNextPlayerAnimation();
}

void GuiWrapper::bettingRoundAnimation(int myBettingRoundID)
{
    myW->signalBettingRoundAnimation2(myBettingRoundID);
}

void GuiWrapper::preflopAnimation1()
{
    myW->signalPreflopAnimation1();
}
void GuiWrapper::preflopAnimation2()
{
    myW->signalPreflopAnimation2();
}

void GuiWrapper::flopAnimation1()
{
    myW->signalFlopAnimation1();
}
void GuiWrapper::flopAnimation2()
{
    myW->signalFlopAnimation2();
}

void GuiWrapper::turnAnimation1()
{
    myW->signalTurnAnimation1();
}
void GuiWrapper::turnAnimation2()
{
    myW->signalTurnAnimation2();
}

void GuiWrapper::riverAnimation1()
{
    myW->signalRiverAnimation1();
}
void GuiWrapper::riverAnimation2()
{
    myW->signalRiverAnimation2();
}

void GuiWrapper::postRiverAnimation1()
{
    myW->signalPostRiverAnimation1();
}
void GuiWrapper::postRiverRunAnimation1()
{
    myW->signalPostRiverRunAnimation1();
}

void GuiWrapper::flipHolecardsAllIn()
{
    myW->signalFlipHolecardsAllIn();
}

void GuiWrapper::nextRoundCleanGui()
{
    myW->signalNextRoundCleanGui();
}

void GuiWrapper::meInAction()
{
    myW->signalMeInAction();
}
void GuiWrapper::showCards(unsigned playerId)
{
    myW->signalPostRiverShowCards(playerId);
}
void GuiWrapper::updateMyButtonsState()
{
    myW->signalUpdateMyButtonsState();
}
void GuiWrapper::disableMyButtons()
{
    myW->signalDisableMyButtons();
}
void GuiWrapper::logPlayerActionMsg(string playerName, int action, int setValue)
{
    myGuiDisplayGameActions->signalLogPlayerActionMsg(QString::fromUtf8(playerName.c_str()), action, setValue);
}
void GuiWrapper::logNewGameHandMsg(int gameID, int handID)
{
    myGuiDisplayGameActions->signalLogNewGameHandMsg(gameID, handID);
}
void GuiWrapper::logNewBlindsSetsMsg(int sbSet, int bbSet, std::string sbName, std::string bbName)
{
    myGuiDisplayGameActions->signalLogNewBlindsSetsMsg(sbSet, bbSet, QString::fromUtf8(sbName.c_str()),
                                                       QString::fromUtf8(bbName.c_str()));
}
void GuiWrapper::logPlayerWinsMsg(std::string playerName, int pot, bool main)
{
    myGuiDisplayGameActions->signalLogPlayerWinsMsg(QString::fromUtf8(playerName.c_str()), pot, main);
}
void GuiWrapper::logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4, int card5)
{
    myGuiDisplayGameActions->signalLogDealBoardCardsMsg(roundID, card1, card2, card3, card4, card5);
}
void GuiWrapper::logFlipHoleCardsMsg(string playerName, int card1, int card2, int cardsValueInt, string showHas)
{
    myGuiDisplayGameActions->signalLogFlipHoleCardsMsg(QString::fromUtf8(playerName.c_str()), card1, card2,
                                                       cardsValueInt, QString::fromUtf8(showHas.c_str()));
}
void GuiWrapper::logPlayerWinGame(std::string playerName, int gameID)
{
    myGuiDisplayGameActions->signalLogPlayerWinGame(QString::fromUtf8(playerName.c_str()), gameID);
}
