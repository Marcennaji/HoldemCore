// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/cards/Card.h"
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

    void setSeatsList(pkt::core::player::PlayerList seats) override
    {
        throw std::runtime_error("Use setSeatsListFsm instead");
    };
    void setActingPlayersList(pkt::core::player::PlayerList actingPlayers) override
    {
        throw std::runtime_error("Use setActingPlayersListFsm instead");
    };
    void setSeatsListFsm(pkt::core::player::PlayerFsmList seats) override;
    void setActingPlayersListFsm(pkt::core::player::PlayerFsmList actingPlayers) override;

    void setCards(int* theValue) override;
    void getCards(int* theValue) override;

    // Modern BoardCards interface (preferred for new code)
    void setBoardCards(const BoardCards& boardCards) override;
    const BoardCards& getBoardCards() const override;

    void setAllInCondition(bool theValue) override;
    void setLastActionPlayerId(unsigned theValue) override;

    int getPot() const override { throw std::runtime_error("getPot is deprecated"); }
    int getPot(const HandFsm& hand) const override;
    void setPot(int theValue) override { throw std::runtime_error("setPot is deprecated"); }
    int getSets() const override { throw std::runtime_error("getSets is deprecated"); }
    int getSets(const HandFsm& hand) const override;

    void collectSets() override { throw std::runtime_error("collectSets is deprecated"); }
    void collectPot() override { throw std::runtime_error("collectPot is deprecated"); }

    void distributePot() override { throw std::runtime_error("distributePot is deprecated"); }

    void distributePot(HandFsm& hand) override;
    void determinePlayerNeedToShowCards() override;

    std::list<unsigned> getWinners() const override;
    void setWinners(const std::list<unsigned>& w) override;

    std::list<unsigned> getPlayerNeedToShowCards() const override;
    void setPlayerNeedToShowCards(const std::list<unsigned>& p) override;

  private:
  private:
    const GameEvents& myEvents;
    pkt::core::player::PlayerFsmList mySeatsList;
    pkt::core::player::PlayerFsmList myActingPlayersList;

    std::list<unsigned> myWinners;
    std::list<unsigned> myPlayerNeedToShowCards;

    BoardCards myBoardCards;
    int myPot{0};
    int myCurrentRoundTotalBets{0};
    unsigned myDealerPlayerId;
    bool myAllInCondition{false};
    unsigned myLastActionPlayerId{0};
};

} // namespace pkt::core
