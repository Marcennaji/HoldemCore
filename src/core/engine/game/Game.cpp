// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Game.h"

#include "EngineFactory.h"
#include "Exception.h"
#include "core/player/Helpers.h"
#include "model/EngineError.h"
#include "model/StartData.h"

#include <iostream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Game::Game(const GameEvents& events, EngineFactory& factory, std::shared_ptr<Board> board,
           PlayerList seatsList, int dealerId, const GameData& gameData, const StartData& startData)
    : m_engineFactory(factory), m_events(events), m_currentBoard(board), m_seatsList(seatsList),
      m_dealerPlayerId(dealerId), m_gameData(gameData), m_startData(startData)
{
    if (!board || !seatsList || seatsList->empty())
        throw Exception(__FILE__, __LINE__, EngineError::MissingParameter);

    // Acting players list starts identical to seats list
    m_actingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>(*m_seatsList);

    // Validate dealer exists
    auto it = getPlayerListIteratorById(m_seatsList, dealerId);
    if (it == m_seatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);
}

void Game::startNewHand()
{
    // Reset the board to preflop state (clear all community cards)
    m_currentBoard->setBoardCards(BoardCards());
    
    m_actingPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>(*m_seatsList);

    // Determine dealer for THIS hand before creating the Hand.
    // For the very first hand, use the dealer provided at Game construction.
    // For subsequent hands, rotate to the next eligible dealer first.
    if (m_currentHand) {
        findNextDealer();
    }

    StartData handStart = m_startData;
    handStart.startDealerPlayerId = m_dealerPlayerId;

    m_currentHand = m_engineFactory.createHand(m_currentBoard, m_seatsList, m_actingPlayersList,
                                                m_gameData, handStart);
    m_currentHand->initialize();
    m_currentHand->runGameLoop();
}

void Game::findNextDealer()
{
    bool nextDealerFound = false;
    // Work off the stable seats list; dealer must be among seated players
    auto dealerPos = getPlayerListIteratorById(m_seatsList, m_dealerPlayerId);

    if (dealerPos == m_seatsList->end())
        throw Exception(__FILE__, __LINE__, EngineError::DealerNotFound);

    for (size_t i = 0; i < m_seatsList->size(); ++i) {
        ++dealerPos;
        if (dealerPos == m_seatsList->end())
            dealerPos = m_seatsList->begin();

        auto playerIt = getPlayerListIteratorById(m_seatsList, (*dealerPos)->getId());
        if (playerIt != m_seatsList->end()) {
            m_dealerPlayerId = (*playerIt)->getId();
            nextDealerFound = true;
            break;
        }
    }

    if (!nextDealerFound)
        throw Exception(__FILE__, __LINE__, EngineError::NextDealerNotFound);
}

void Game::handlePlayerAction(const PlayerAction& action)
{
    if (m_currentHand) {
        m_currentHand->handlePlayerAction(action);
    }
}

} // namespace pkt::core
