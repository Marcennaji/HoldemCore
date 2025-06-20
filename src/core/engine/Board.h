// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "core/interfaces/IBoard.h"

#include "core/player/typedefs.h"

namespace pkt::core::player
{
class Player;

} // namespace pkt::core::player
namespace pkt::core
{

class IHand;

class Board : public IBoard
{
  public:
    Board(unsigned dealerPosition);
    ~Board();

    void setPlayerLists(pkt::core::player::PlayerList, pkt::core::player::PlayerList, pkt::core::player::PlayerList);

    void setCards(int* theValue);
    void getCards(int* theValue);

    void setAllInCondition(bool theValue);
    void setLastActionPlayerId(unsigned theValue);

    int getPot() const;
    void setPot(int theValue);
    int getSets() const;
    void setSets(int theValue);

    void collectSets();
    void collectPot();

    void distributePot();
    void determinePlayerNeedToShowCards();

    std::list<unsigned> getWinners() const;
    void setWinners(const std::list<unsigned>& w);

    std::list<unsigned> getPlayerNeedToShowCards() const;
    void setPlayerNeedToShowCards(const std::list<unsigned>& p);

  private:
    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myActivePlayerList;
    pkt::core::player::PlayerList myRunningPlayerList;

    std::list<unsigned> myWinners;
    std::list<unsigned> myPlayerNeedToShowCards;

    int myCards[5];
    int myPot;
    int mySets;
    unsigned myDealerPosition;
    bool myAllInCondition;
    unsigned myLastActionPlayerId;
};

} // namespace pkt::core
