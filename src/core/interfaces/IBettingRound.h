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

    virtual void setCurrentPlayersTurnId(unsigned) = 0;
    virtual unsigned getCurrentPlayersTurnId() const = 0;

    virtual void setCurrentPlayersTurnIt(pkt::core::player::PlayerListIterator) = 0;
    virtual pkt::core::player::PlayerListIterator getCurrentPlayersTurnIt() const = 0;

    virtual unsigned getSmallBlindPlayerId() const = 0;

    virtual unsigned getBigBlindPlayerId() const = 0;

    virtual void setHighestSet(int) = 0;
    virtual int getHighestSet() const = 0;

    virtual void setHighestCardsValue(int theValue) = 0;
    virtual int getHighestCardsValue() const = 0;

    virtual void setMinimumRaise(int) = 0;
    virtual int getMinimumRaise() const = 0;

    virtual void setFullBetRule(bool) = 0;
    virtual bool getFullBetRule() const = 0;

    virtual bool getFirstRound() const = 0;

    virtual void skipFirstRunGui() = 0;

    virtual void nextPlayer() = 0;
    virtual void run() = 0;

    virtual void postRiverRun() = 0;
};
} // namespace pkt::core
