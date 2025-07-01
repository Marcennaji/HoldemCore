// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "Player.h"

#include "core/player/strategy/IBotStrategy.h"

namespace pkt::core::player
{
class BotPlayer : public Player
{

  public:
    BotPlayer(const GameEvents&, int id, std::string name, int sC, bool aS, int mB);

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
    int myCurrentHandID = 0;
    int myBetAmount = 0;
    int myRaiseAmount = 0;
};

} // namespace pkt::core::player
