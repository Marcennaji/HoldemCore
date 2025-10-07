#pragma once

#include <vector>
#include <memory>
#include "core/engine/cards/Card.h"
#include "core/interfaces/Randomizer.h"

namespace pkt::core
{

/// DeckManager handles pure deck operations for a poker hand
/// Extracted from Hand class to separate deck management from game logic  
/// ISP-compliant: only depends on Randomizer interface
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