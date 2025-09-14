// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/engine/GameEvents.h"
#include "core/interfaces/IBoard.h"
#include "core/player/typedefs.h"

#include <iostream>
#include <memory>
#include <vector>

namespace pkt::core::player
{
class Player;

} // namespace pkt::core::player
namespace pkt::core
{

class HandFsm;

class BoardFsm : public IBoard
{
  public:
    BoardFsm(unsigned dealerPosition, const GameEvents& events);
    ~BoardFsm();

    void setSeatsList(pkt::core::player::PlayerList seats) { throw std::runtime_error("Use setSeatsListFsm instead"); };
    void setActingPlayersList(pkt::core::player::PlayerList actingPlayers)
    {
        throw std::runtime_error("Use setActingPlayersListFsm instead");
    };
    void setSeatsListFsm(pkt::core::player::PlayerFsmList seats);
    void setActingPlayersListFsm(pkt::core::player::PlayerFsmList actingPlayers);

    void setCards(int* theValue);
    void getCards(int* theValue);

    void setAllInCondition(bool theValue);
    void setLastActionPlayerId(unsigned theValue);

    int getPot() const { throw std::runtime_error("getPot is deprecated"); }
    int getPot(const HandFsm& hand) const;
    void setPot(int theValue) { throw std::runtime_error("setPot is deprecated"); }
    int getSets() const { throw std::runtime_error("getSets is deprecated"); }
    int getSets(const HandFsm& hand) const;

    void collectSets() { throw std::runtime_error("collectSets is deprecated"); }
    void collectPot() { throw std::runtime_error("collectPot is deprecated"); }

    void distributePot() { throw std::runtime_error("distributePot is deprecated"); }

    void distributePot(HandFsm& hand);
    void determinePlayerNeedToShowCards();

    std::list<unsigned> getWinners() const;
    void setWinners(const std::list<unsigned>& w);

    std::list<unsigned> getPlayerNeedToShowCards() const;
    void setPlayerNeedToShowCards(const std::list<unsigned>& p);

  private:
  private:
    const GameEvents& myEvents;
    pkt::core::player::PlayerFsmList mySeatsList;
    pkt::core::player::PlayerFsmList myActingPlayersList;

    std::list<unsigned> myWinners;
    std::list<unsigned> myPlayerNeedToShowCards;

    int myCards[5];
    int myPot{0};
    int myCurrentRoundTotalBets{0};
    unsigned myDealerPlayerId;
    bool myAllInCondition{false};
    unsigned myLastActionPlayerId{0};
};

} // namespace pkt::core
