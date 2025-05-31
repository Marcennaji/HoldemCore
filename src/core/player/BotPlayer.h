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

#pragma once

#include "Player.h"
#include "core/player/strategy/CurrentHandContext.h"
#include "core/player/strategy/IBotStrategy.h"

namespace pkt::core
{
class BotPlayer : public Player
{

  public:
    BotPlayer(GameEvents*, IHandAuditStore*, IPlayersStatisticsStore*, int id, PlayerType type, std::string name,
              int sC, bool aS, bool sotS, int mB);

    ~BotPlayer();

    void setStrategy(std::unique_ptr<IBotStrategy> strategy) { myStrategy = std::move(strategy); }
    void action();
    void doPreflopAction();
    void doFlopAction();
    void doTurnAction();
    void doRiverAction();

  private:
    void evaluateBetAmount();
    void updatePlayerContext(const GameState gameState);

    std::unique_ptr<IBotStrategy> myStrategy;
    GameState myCurrentGameState = GAME_STATE_NONE;
    int myBetAmount = 0;
    int myRaiseAmount = 0;
    CurrentHandContext myCurrentHandContext;
};

} // namespace pkt::core
