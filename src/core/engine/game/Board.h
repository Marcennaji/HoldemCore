// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/engine/GameEvents.h"
#include "core/engine/cards/Card.h"
#include "core/interfaces/IBoard.h"
#include "core/player/typedefs.h"
#include "core/services/ServiceContainer.h"

#include <iostream>
#include <memory>
#include <vector>

namespace pkt::core::player
{
class Player;

} // namespace pkt::core::player
namespace pkt::core
{

class Hand;

class Board : public IBoard
{
  public:
    Board(unsigned dealerPosition, const GameEvents& events);
    Board(unsigned dealerPosition, const GameEvents& events, std::shared_ptr<ServiceContainer> services);
    ~Board();

    void setSeatsList(pkt::core::player::PlayerList seats) override;
    void setActingPlayersList(pkt::core::player::PlayerList actingPlayers) override;

    // Modern BoardCards interface (preferred for new code)
    void setBoardCards(const BoardCards& boardCards) override;
    const BoardCards& getBoardCards() const override;

    void setAllInCondition(bool theValue) override;
    void setLastActionPlayerId(unsigned theValue) override;

    int getPot() const override { throw std::runtime_error("getPot is deprecated"); }
    int getPot(const Hand& hand) const override;
    void setPot(int theValue) override { throw std::runtime_error("setPot is deprecated"); }
    int getSets() const override { throw std::runtime_error("getSets is deprecated"); }
    int getSets(const Hand& hand) const override;

    void collectSets() override { throw std::runtime_error("collectSets is deprecated"); }
    void collectPot() override { throw std::runtime_error("collectPot is deprecated"); }

    void distributePot() override { throw std::runtime_error("distributePot is deprecated"); }

  void distributePot(Hand& hand) override;
  void determineShowdownRevealOrder() override;

    std::list<unsigned> getWinners() const override;
    void setWinners(const std::list<unsigned>& w) override;

  std::vector<unsigned> getShowdownRevealOrder() const override { return myShowdownRevealOrder; }

  private:
    void ensureServicesInitialized() const;
  private:
    const GameEvents& myEvents;
    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myActingPlayersList;

    std::list<unsigned> myWinners;
  std::vector<unsigned> myShowdownRevealOrder; // ordered, deduplicated, preserves reveal sequence

    BoardCards myBoardCards;
    int myPot{0};
    int myCurrentRoundTotalBets{0};
    unsigned myDealerPlayerId;
    bool myAllInCondition{false};
    unsigned myLastActionPlayerId{0};
  mutable std::shared_ptr<ServiceContainer> myServices;
};

} // namespace pkt::core
