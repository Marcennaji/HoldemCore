#pragma once

#include <vector>
#include "core/engine/cards/Card.h"

namespace pkt::core
{

/// DeckManager handles pure deck operations for a poker hand
/// Extracted from Hand class to separate deck management from game logic
class DeckManager
{
  public:
    DeckManager();
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
};

} // namespace pkt::core