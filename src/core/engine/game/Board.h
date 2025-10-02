// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/engine/GameEvents.h"
#include "core/engine/cards/Card.h"
#include "core/engine/game/Board.h"
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

class Board 
{
  public:
    Board(unsigned dealerPosition, const GameEvents& events);
    Board(unsigned dealerPosition, const GameEvents& events, std::shared_ptr<ServiceContainer> services);
    ~Board();

    void setSeatsList(pkt::core::player::PlayerList seats);
    void setActingPlayersList(pkt::core::player::PlayerList actingPlayers);

    // Modern BoardCards interface (preferred for new code)
    void setBoardCards(const BoardCards& boardCards);
    const BoardCards& getBoardCards() const;

    void setAllInCondition(bool theValue);
    void setLastActionPlayerId(unsigned theValue);

    int getPot(const Hand& hand) const;
    int getSets(const Hand& hand) const;

    void distributePot(Hand& hand);
    void determineShowdownRevealOrder();

    std::list<unsigned> getWinners() const;
    void setWinners(const std::list<unsigned>& w);

    std::vector<unsigned> getShowdownRevealOrder() const { return myShowdownRevealOrder; }

  private:
    void ensureServicesInitialized() const;
    const GameEvents& myEvents;
    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myActingPlayersList;

    std::list<unsigned> myWinners;
    std::vector<unsigned> myShowdownRevealOrder; // ordered, deduplicated, preserves reveal sequence

    BoardCards myBoardCards;
    int myPot{0};
    int myCurrentRoundTotalBets{0};
    unsigned myDealerPlayerId{0};
    bool myAllInCondition{false};
    unsigned myLastActionPlayerId{0};
  mutable std::shared_ptr<ServiceContainer> myServices;
};

} // namespace pkt::core
