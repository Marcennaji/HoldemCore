// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "core/cards/Card.h"
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

    void setSeatsList(pkt::core::player::PlayerList seats) override;
    void setActingPlayersList(pkt::core::player::PlayerList actingPlayers) override;
    void setSeatsListFsm(pkt::core::player::PlayerFsmList seats) override {};
    void setActingPlayersListFsm(pkt::core::player::PlayerFsmList actingPlayers) override {};

    void setCards(int* theValue) override;
    void getCards(int* theValue) override;

    // Modern BoardCards interface (not implemented in deprecated class)
    void setBoardCards(const BoardCards& boardCards) override
    {
        throw std::runtime_error("setBoardCards not implemented in deprecated Board class");
    }
    const BoardCards& getBoardCards() const override
    {
        throw std::runtime_error("getBoardCards not implemented in deprecated Board class");
    }

    void setAllInCondition(bool theValue) override;
    void setLastActionPlayerId(unsigned theValue) override;

    int getPot() const override;
    int getPot(const HandFsm& hand) const override { throw std::runtime_error("getPot Not implemented"); }
    void setPot(int theValue) override;
    int getSets() const override;
    int getSets(const HandFsm& hand) const override { throw std::runtime_error("getSets Not implemented"); }

    void collectSets() override;
    void collectPot() override;

    void distributePot() override;
    void determinePlayerNeedToShowCards() override;

    std::list<unsigned> getWinners() const override;
    void setWinners(const std::list<unsigned>& w) override;

    std::list<unsigned> getPlayerNeedToShowCards() const override;
    void setPlayerNeedToShowCards(const std::list<unsigned>& p) override;

  private:
    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myActingPlayersList;

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
