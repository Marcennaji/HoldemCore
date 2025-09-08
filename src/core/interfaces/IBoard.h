// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"

#include <list>
namespace pkt::core
{

class IHand;

class IBoard
{

  public:
    virtual ~IBoard() = default;
    //
    virtual void setSeatsList(pkt::core::player::PlayerList seats) = 0;
    virtual void setActingPlayersList(pkt::core::player::PlayerList actingPlayers) = 0;
    virtual void setSeatsListFsm(pkt::core::player::PlayerFsmList seats) = 0;
    virtual void setActingPlayersListFsm(pkt::core::player::PlayerFsmList actingPlayers) = 0;
    //
    virtual void setCards(int* theValue) = 0;
    virtual void getCards(int* theValue) = 0;
    //
    virtual int getPot() const = 0;
    virtual void setPot(int theValue) = 0;
    virtual int getSets() const = 0;
    virtual void setSets(int theValue) = 0;

    virtual void setAllInCondition(bool theValue) = 0;
    virtual void setLastActionPlayerId(unsigned theValue) = 0;
    //
    virtual void collectSets() = 0;
    virtual void collectPot() = 0;

    virtual void distributePot() = 0;
    virtual void determinePlayerNeedToShowCards() = 0;

    virtual std::list<unsigned> getWinners() const = 0;
    virtual void setWinners(const ::std::list<unsigned>& winners) = 0;

    virtual std::list<unsigned> getPlayerNeedToShowCards() const = 0;
    virtual void setPlayerNeedToShowCards(const ::std::list<unsigned>& playerNeedToShowCards) = 0;
};

} // namespace pkt::core
