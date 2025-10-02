#pragma once

namespace pkt::core
{

class DeckDealer
{
  public:
    virtual ~DeckDealer() = default;
    virtual void initAndShuffleDeck() = 0;
    virtual void dealHoleCards(size_t lastArrayIndex) = 0;
    virtual size_t dealBoardCards() = 0;
};

} // namespace pkt::core