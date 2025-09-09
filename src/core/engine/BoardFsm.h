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

class BoardFsm : public IBoard
{
  public:
    BoardFsm(unsigned dealerPosition);
    ~BoardFsm();

    void setSeatsList(pkt::core::player::PlayerFsmList seats) {};
    void setActingPlayersList(pkt::core::player::PlayerFsmList actingPlayers) {};
    void setSeatsListFsm(pkt::core::player::PlayerFsmList seats);
    void setActingPlayersListFsm(pkt::core::player::PlayerFsmList actingPlayers);

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
    pkt::core::player::PlayerFsmList mySeatsList;
    pkt::core::player::PlayerFsmList myActingPlayersList;

    std::list<unsigned> myWinners;
    std::list<unsigned> myPlayerNeedToShowCards;

    int myCards[5];
    int myPot{0};
    int myTotalBetAmounts{0};
    unsigned myDealerPlayerId;
    bool myAllInCondition{false};
    unsigned myLastActionPlayerId{0};
};

} // namespace pkt::core
