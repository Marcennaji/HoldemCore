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

#include <core/player/typedefs.h>
#include "core/engine/EngineDefs.h"

#include <list>
namespace pkt::core
{

class IHand;

class IBoard
{

  public:
    virtual ~IBoard() {};
    //
    virtual void setPlayerLists(pkt::core::player::PlayerList, pkt::core::player::PlayerList,
                                pkt::core::player::PlayerList) = 0;
    //
    virtual void setCards(int* theValue) = 0;
    virtual void getCards(int* theValue) = 0;
    //
    virtual int getPot() const = 0;
    virtual void setPot(int theValue) = 0;
    virtual int getSets() const = 0;
    virtual void setSets(int theValue) = 0;

    virtual void setAllInCondition(bool theValue) = 0;
    virtual void setLastActionPlayerID(unsigned theValue) = 0;
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
