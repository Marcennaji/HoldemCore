#include "DeckManager.h"
#include <stdexcept>

namespace pkt::core
{

DeckManager::DeckManager() = default;

void DeckManager::initializeAndShuffle()
{
    myDeck.initializeFullDeck();
    myDeck.shuffle();
}

std::vector<Card> DeckManager::dealCards(int numCards)
{
    return myDeck.dealCards(numCards);
}

size_t DeckManager::remainingCards() const
{
    return myDeck.remainingCards();
}

bool DeckManager::hasEnoughCards(size_t boardCards, size_t numPlayers) const
{
    return myDeck.hasEnoughCards(boardCards, numPlayers);
}

void DeckManager::resetDealPosition()
{
    myDeck.resetDealPosition();
}

} // namespace pkt::core