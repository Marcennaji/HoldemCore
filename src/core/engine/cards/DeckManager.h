// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <vector>
#include <memory>
#include "core/engine/cards/Card.h"
#include "core/interfaces/Randomizer.h"

namespace pkt::core
{

/**
 * @brief Manages deck initialization, shuffling, and card dealing operations.
 * 
 * Handles all deck-related operations for poker games including shuffling,
 * dealing cards to players and board, and tracking remaining cards.
 * Follows ISP principles by depending only on the Randomizer interface.
 */

class DeckManager
{
  public:
    DeckManager(Randomizer& randomizer);
    ~DeckManager() = default;

    // Core deck operations
    void initializeAndShuffle();
    std::vector<Card> dealCards(int numCards);

    // Deck state queries
    size_t remainingCards() const;
    bool hasEnoughCards(size_t boardCards, size_t numPlayers) const;

    // Reset deck state
    void resetDealPosition();

  private:
    Deck m_deck;
    Randomizer* m_randomizer;
};

} // namespace pkt::core