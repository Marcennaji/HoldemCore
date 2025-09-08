// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
#include "core/engine/model/GameState.h"

namespace pkt::core
{

class IBettingRound
{
  public:
    virtual ~IBettingRound() = default;

    virtual GameState getBettingRoundId() const = 0;

    virtual void setCurrentPlayerTurnId(int) = 0;
    virtual int getCurrentPlayerTurnId() const = 0;

    virtual int getSmallBlindPlayerId() const = 0;

    virtual int getBigBlindPlayerId() const = 0;

    virtual void setHighestSet(int) = 0;
    virtual int getRoundHighestSet() const = 0;

    virtual void setMinimumRaise(int) = 0;
    virtual int getMinimumRaise() const = 0;

    virtual void setFullBetRule(bool) = 0;
    virtual bool getFullBetRule() const = 0;

    virtual bool getFirstRound() const = 0;

    virtual void giveActionToNextBotPlayer() = 0;
    virtual void run() = 0;
};
} // namespace pkt::core
