// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"
#include "core/engine/cards/Card.h"

#include <list>
#include <stdexcept>
namespace pkt::core
{

class IHand;
class Hand;

class IBoard
{

  public:
    virtual ~IBoard() = default;

    virtual void setSeatsList(pkt::core::player::PlayerList seats) = 0;
    virtual void setActingPlayersList(pkt::core::player::PlayerList actingPlayers) = 0;

    virtual void setBoardCards(const BoardCards& boardCards) = 0;
    virtual const BoardCards& getBoardCards() const = 0;

    virtual int getPot(const Hand& hand) const = 0;
    virtual int getSets(const Hand& hand) const = 0;

    virtual void setAllInCondition(bool theValue) = 0;
    virtual void setLastActionPlayerId(unsigned theValue) = 0;

    virtual void distributePot(Hand& hand) = 0;
 
    // Compute the ordered list of player IDs who must reveal at showdown (domain logic)
    virtual void determineShowdownRevealOrder() = 0;

    virtual std::list<unsigned> getWinners() const = 0;
    virtual void setWinners(const ::std::list<unsigned>& winners) = 0;
  
    // Ordered vector accessor for UI consumption
    virtual std::vector<unsigned> getShowdownRevealOrder() const = 0;
};

} // namespace pkt::core
