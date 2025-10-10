// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

namespace pkt::core
{

/**
 * @brief Abstract interface for dealing cards during poker hands.
 * 
 * Defines the contract for deck management and card dealing operations,
 * including shuffling, dealing hole cards to players, and dealing
 * community cards to the board.
 */
class DeckDealer
{
  public:
    virtual ~DeckDealer() = default;
    virtual void initAndShuffleDeck() = 0;
    virtual void dealHoleCards(size_t lastArrayIndex) = 0;
    virtual size_t dealBoardCards() = 0;
};

} // namespace pkt::core