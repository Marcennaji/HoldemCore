// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/player/strategy/IBotStrategy.h"

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;
class LooseAggressiveBotStrategy : public IBotStrategy
{

  public:
    LooseAggressiveBotStrategy();

    ~LooseAggressiveBotStrategy();

    bool preflopShouldCall(CurrentHandContext& context, bool deterministic = false) override;
    bool flopShouldCall(CurrentHandContext& context, bool deterministic = false) override;
    bool turnShouldCall(CurrentHandContext& context, bool deterministic = false) override;
    bool riverShouldCall(CurrentHandContext& context, bool deterministic = false) override;

    int preflopShouldRaise(CurrentHandContext& context, bool deterministic = false) override;
    int flopShouldRaise(CurrentHandContext& context, bool deterministic = false) override;
    int turnShouldRaise(CurrentHandContext& context, bool deterministic = false) override;
    int riverShouldRaise(CurrentHandContext& context, bool deterministic = false) override;

    int flopShouldBet(CurrentHandContext& context, bool deterministic = false) override;
    int turnShouldBet(CurrentHandContext& context, bool deterministic = false) override;
    int riverShouldBet(CurrentHandContext& context, bool deterministic = false) override;
};

} // namespace pkt::core::player
