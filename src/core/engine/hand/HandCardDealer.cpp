// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "HandCardDealer.h"
#include "core/engine/game/Board.h"
#include "core/player/Player.h"
#include "core/interfaces/Logger.h"
#include <stdexcept>

namespace pkt::core
{

HandCardDealer::HandCardDealer(std::unique_ptr<DeckManager> deckManager,
                               const GameEvents& events,
                               Logger& logger,
                               HandEvaluationEngine& handEvaluationEngine)
    : m_deckManager(std::move(deckManager)), m_events(events), 
      m_logger(&logger), m_handEvaluationEngine(&handEvaluationEngine)
{
}

void HandCardDealer::initializeAndShuffleDeck()
{
    m_deckManager->initializeAndShuffle();
}

void HandCardDealer::dealHoleCards(player::PlayerList actingPlayers, const Board& board)
{
    if (!m_deckManager->hasEnoughCards(0, actingPlayers->size()))
    {
        throw std::runtime_error("Not enough cards in the deck to deal hole cards to all players.");
    }

    for (auto it = actingPlayers->begin(); it != actingPlayers->end(); ++it)
    {
        std::vector<Card> holeCardList = m_deckManager->dealCards(2);

        setPlayerHoleCardsAndRanking(*it, holeCardList, board);

        // Fire event for UI notification
        if (m_events.onHoleCardsDealt)
        {
            HoleCards holeCards(holeCardList[0], holeCardList[1]);
            m_events.onHoleCardsDealt((*it)->getId(), holeCards);
        }
    }
}

size_t HandCardDealer::dealBoardCards(Board& board)
{
    std::vector<Card> boardCardList = m_deckManager->dealCards(5);

    BoardCards boardCards;
    
    // Deal all board cards at once (flop, turn, river)
    boardCards.dealFlop(boardCardList[0], boardCardList[1], boardCardList[2]);
    boardCards.dealTurn(boardCardList[3]);
    boardCards.dealRiver(boardCardList[4]);

    board.setBoardCards(boardCards);

    return 5;
}

std::vector<Card> HandCardDealer::dealCardsFromDeck(int numCards)
{
    return m_deckManager->dealCards(numCards);
}

bool HandCardDealer::hasEnoughCardsForPlayers(size_t numPlayers) const
{
    return m_deckManager->hasEnoughCards(0, numPlayers);
}

size_t HandCardDealer::remainingCards() const
{
    return m_deckManager->remainingCards();
}

void HandCardDealer::setPlayerHoleCardsAndRanking(std::shared_ptr<player::Player> player, 
                                                 const std::vector<Card>& holeCardList, 
                                                 const Board& board)
{
    // Set the hole cards for the player
    HoleCards holeCards(holeCardList[0], holeCardList[1]);
    player->setHoleCards(holeCards);

    // Build hand evaluation string and set player ranking
    std::string humanReadableHand = buildHandEvaluationString(holeCardList, board);
    player->setHandRanking(m_handEvaluationEngine->rankHand(humanReadableHand.c_str()));
}

std::string HandCardDealer::buildHandEvaluationString(const std::vector<Card>& holeCardList, 
                                                     const Board& board) const
{
    // Build evaluator string with correct ordering: HOLE cards first, then current BOARD cards
    std::string humanReadableHand = holeCardList[0].toString() + std::string(" ") + holeCardList[1].toString();
    
    const BoardCards& boardCards = board.getBoardCards();
    if (boardCards.getNumCards() > 0)
    {
        humanReadableHand += std::string(" ") + boardCards.toString();
    }
    
    return humanReadableHand;
}

} // namespace pkt::core