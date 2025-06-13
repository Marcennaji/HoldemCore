/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#pragma once

#include "Player.h"

#include "core/player/strategy/IBotStrategy.h"

namespace pkt::core::player
{
class BotPlayer : public Player
{

  public:
    BotPlayer(GameEvents*, IHandAuditStore*, IPlayersStatisticsStore*, int id, std::string name, int sC, bool aS,
              int mB);

    ~BotPlayer();

    void setStrategy(std::unique_ptr<IBotStrategy> strategy) { myStrategy = std::move(strategy); }
    void action();
    void doPreflopAction();
    void doFlopAction();
    void doTurnAction();
    void doRiverAction();
    virtual std::string getStrategyName() const override
    {
        return myStrategy ? myStrategy->getStrategyName() : "Unknown Strategy";
    }

    float calculatePreflopCallingRange(CurrentHandContext& context, bool deterministic) const override;

  private:
    void evaluateBetAmount();

    std::unique_ptr<IBotStrategy> myStrategy;
    GameState myCurrentGameState = GAME_STATE_NONE;
    int myCurrentHandID = 0;
    int myBetAmount = 0;
    int myRaiseAmount = 0;
};

} // namespace pkt::core::player
