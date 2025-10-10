// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include "core/engine/GameEvents.h"
#include "core/engine/cards/Card.h"
#include "core/engine/game/Board.h"
#include "core/player/typedefs.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/HandEvaluationEngine.h"

#include <iostream>
#include <memory>
#include <vector>

namespace pkt::core
{

class Hand;

class Board 
{
  public:

    Board(unsigned dealerPosition, const GameEvents& events,
          Logger& logger, HandEvaluationEngine& handEvaluator);
          
    ~Board();

    void setSeatsList(pkt::core::player::PlayerList seats);
    void setActingPlayersList(pkt::core::player::PlayerList actingPlayers);

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

    std::vector<unsigned> getShowdownRevealOrder() const { return m_showdownRevealOrder; }
    
    std::string getStringRepresentation() const;

  private:
    const GameEvents& m_events;
    pkt::core::player::PlayerList m_seatsList;
    pkt::core::player::PlayerList m_actingPlayersList;

    std::list<unsigned> m_winners;
    std::vector<unsigned> m_showdownRevealOrder; // ordered, deduplicated, preserves reveal sequence

    BoardCards m_boardCards;
    int m_pot{0};
    int m_currentRoundTotalBets{0};
    unsigned m_dealerPlayerId{0};
    bool m_allInCondition{false};
    unsigned m_lastActionPlayerId{0};
    
    Logger* m_logger;
    HandEvaluationEngine* m_handEvaluator;
};

} // namespace pkt::core
