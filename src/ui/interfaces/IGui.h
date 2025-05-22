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
#ifndef GUIINTERFACE_H
#define GUIINTERFACE_H

#include <core/engine/model/GameState.h>

#include <memory>
#include <string>

class Session;

class IGui
{
  public:
    virtual ~IGui() {};

    virtual void initGui(int speed) = 0;

    virtual void* getGameTableWindow() const = 0;

    // virtual void hideHoleCards() = 0;
    virtual void setStartWindow(void*) = 0;
    // virtual void refreshSet() const = 0;
    // virtual void refreshCash() const = 0;
    // virtual void refreshAction(int = -1, int = -1) const = 0;
    // virtual void refreshPot() const = 0;
    // virtual void refreshPlayersActiveInactiveStyles(int = -1, int = -1) const = 0;
    // virtual void nextBettingRoundInitializeGui() = 0;
    // virtual void refreshAll() const = 0;
    // virtual void refreshPlayerName() const = 0;
    // virtual void refreshDealerAndBlindsButtons() const = 0;
    // virtual void refreshTableDescriptiveLabels(GameState state) const = 0;

    // virtual void waitForGuiUpdateDone() const = 0;

    // virtual void dealBettingRoundCards(int) = 0;
    // virtual void dealHoleCards() = 0;
    // virtual void dealFlopCards() = 0;
    // virtual void dealTurnCard() = 0;
    // virtual void dealRiverCard() = 0;

    // virtual void activePlayerActionDone() = 0;
    // virtual void doHumanAction() = 0;
    // virtual void showCards(unsigned playerId) = 0;
    // virtual void updateHumanPlayerButtonsState() = 0;
    virtual void disableMyButtons() = 0;
    virtual void bettingRoundAnimation(int) = 0;

    virtual void preflopAnimation1() = 0;
    virtual void preflopAnimation2() = 0;

    virtual void flopAnimation1() = 0;
    virtual void flopAnimation2() = 0;

    virtual void turnAnimation1() = 0;
    virtual void turnAnimation2() = 0;

    virtual void riverAnimation1() = 0;
    virtual void riverAnimation2() = 0;

    virtual void postRiverAnimation1() = 0;
    virtual void postRiverRunAnimation1() = 0;
    // virtual void flipHoleCardsAllIn() = 0;
};

#endif
