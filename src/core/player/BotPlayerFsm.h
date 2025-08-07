// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "PlayerFsm.h"

#include "core/player/strategy/IBotStrategy.h"

namespace pkt::core::player
{
class BotPlayerFsm : public PlayerFsm
{

  public:
    BotPlayerFsm(const GameEvents&, int id, std::string name, int cash);

    ~BotPlayerFsm();

    void setStrategy(std::unique_ptr<IBotStrategy> strategy) { myStrategy = std::move(strategy); }
    virtual std::string getStrategyName() const override
    {
        return myStrategy ? myStrategy->getStrategyName() : "Unknown Strategy";
    }

    PlayerAction decidePreflopAction();

    bool isBot() const override { return true; }

  private:
    std::unique_ptr<IBotStrategy> myStrategy;
};

} // namespace pkt::core::player
