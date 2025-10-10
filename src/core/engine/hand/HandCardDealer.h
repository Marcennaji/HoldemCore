// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/cards/Card.h"
#include "core/engine/cards/DeckManager.h"
#include "core/engine/GameEvents.h"
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/Logger.h"
#include "core/player/typedefs.h"
#include <memory>
#include <vector>

namespace pkt::core
{

// Forward declarations
class Board;
namespace player { 
    class Player; 
}

/**
 * @brief Handles all card dealing operations for a poker hand.
 * 
 * This class extracts card dealing responsibilities from the Hand class,
 * implementing the DeckDealer interface through composition rather than inheritance.
 * 
 * Responsibilities:
 * - Deck initialization and shuffling
 * - Dealing hole cards to players
 * - Dealing board cards (flop, turn, river)
 * - Managing card distribution logic
 * - Coordinating with hand evaluation for player rankings
 */
class HandCardDealer
{
  public:
    HandCardDealer(std::unique_ptr<DeckManager> deckManager,
                   const GameEvents& events,
                   Logger& logger,
                   HandEvaluationEngine& handEvaluationEngine);
    
    ~HandCardDealer() = default;

    // Core card dealing operations (matches DeckDealer interface)
    void initializeAndShuffleDeck();
    void dealHoleCards(player::PlayerList actingPlayers, const Board& board);
    size_t dealBoardCards(Board& board);

    // Additional card operations  
    std::vector<Card> dealCardsFromDeck(int numCards);

    // Deck state queries
    bool hasEnoughCardsForPlayers(size_t numPlayers) const;
    size_t remainingCards() const;

  private:
    std::unique_ptr<DeckManager> m_deckManager;
    const GameEvents& m_events;
    Logger* m_logger;
    HandEvaluationEngine* m_handEvaluationEngine;

    // Helper methods
    void setPlayerHoleCardsAndRanking(std::shared_ptr<player::Player> player, 
                                     const std::vector<Card>& holeCardList, 
                                     const Board& board);
    std::string buildHandEvaluationString(const std::vector<Card>& holeCardList, 
                                        const Board& board) const;
};

} // namespace pkt::core