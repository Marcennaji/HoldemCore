// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include "core/engine/model/PlayerAction.h"
#include "core/player/Player.h"

#include <memory>

namespace pkt::core
{
class Hand;
class HandState;

class HandActionProcessor
{
  public:
    virtual ~HandActionProcessor() = default;

    virtual void promptPlayerAction(Hand& hand, player::Player& player) = 0;
    virtual std::unique_ptr<HandState> computeNextState(Hand& hand) = 0;
    virtual bool isActionAllowed(const Hand& hand, PlayerAction action) const = 0;

    virtual std::shared_ptr<player::Player> getNextPlayerToAct(const Hand& hand) const = 0;
    virtual std::shared_ptr<player::Player> getFirstPlayerToActInRound(const Hand& hand) const = 0;
    virtual bool isRoundComplete(const Hand& hand) const = 0;
};
} // namespace pkt::core
