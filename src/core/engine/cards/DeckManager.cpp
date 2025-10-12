#include "DeckManager.h"
#include <stdexcept>

namespace pkt::core
{

DeckManager::DeckManager(Randomizer& randomizer) 
    : m_randomizer(randomizer) {}

void DeckManager::initializeAndShuffle()
{
    m_deck.initializeFullDeck();
    m_deck.shuffle(m_randomizer);
}

std::vector<Card> DeckManager::dealCards(int numCards)
{
    return m_deck.dealCards(numCards);
}

size_t DeckManager::remainingCards() const
{
    return m_deck.remainingCards();
}

bool DeckManager::hasEnoughCards(size_t boardCards, size_t numPlayers) const
{
    return m_deck.hasEnoughCards(boardCards, numPlayers);
}

void DeckManager::resetDealPosition()
{
    m_deck.resetDealPosition();
}

} // namespace pkt::core