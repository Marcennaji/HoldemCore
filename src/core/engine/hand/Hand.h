// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <map>
#include <memory>
#include "../actions/ActionValidator.h"
#include "../actions/InvalidActionHandler.h"
#include "../cards/Card.h"
#include "Player.h"
#include "core/engine/cards/DeckManager.h"
#include "core/engine/hand/HandPlayersState.h"
#include "core/engine/hand/HandPlayersManager.h"
#include "core/engine/hand/HandActionHandler.h"
#include "core/engine/hand/HandStateManager.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/engine/hand/ActionProcessor.h"
#include "core/engine/hand/HandCardDealer.h"
#include "core/engine/hand/HandCalculator.h"
#include "core/engine/hand/HandLifecycleManager.h"
#include "core/engine/hand/HandPlayerAction.h"
#include "core/engine/hand/HandState.h"

#include "strategy/CurrentHandContext.h"

namespace pkt::core
{

class HandState;
class EngineFactory;
class Board;

/**
 * @brief Coordinator class for a single poker hand, orchestrating all hand-related operations through composition.
 * 
 * The Hand class serves as the central coordinator for a poker hand, implementing the SOLID principles
 * through composition over inheritance. It delegates responsibilities to specialized component managers:
 * 
 * Architecture:
 * - HandPlayersManager: Manages player collections, positions, and player-related state
 * - HandActionHandler: Processes and validates player actions with error handling
 * - HandCardDealer: Handles deck management and card distribution operations
 * - HandCalculator: Performs hand-related calculations (pot odds, M-ratio, etc.)
 * - HandStateManager: Manages game state transitions and the game loop
 * - HandLifecycleManager: Handles hand initialization, execution, and cleanup
 * 
 * The Hand class maintains minimal state and serves as a clean interface that coordinates
 * between these components, ensuring proper separation of concerns and maintainability.
 * This design follows the Composition over Inheritance principle and implements the
 * Coordinator pattern for complex domain orchestration.
 * 
 * Usage:
 * 1. Create Hand instance with required dependencies
 * 2. Call initialize() to set up the hand
 * 3. Call runGameLoop() to execute the hand logic
 * 4. Call end() to finalize and clean up
 * 
 * Thread Safety: Not thread-safe; designed for single-threaded game execution.
 * 
 * @see HandPlayersManager, HandActionHandler, HandCardDealer, HandCalculator, 
 *      HandStateManager, HandLifecycleManager
 */
class Hand
{
  public:

    Hand(const GameEvents&, std::shared_ptr<Board>,
         pkt::core::player::PlayerList seats, pkt::core::player::PlayerList actingPlayers, GameData gameData,
         StartData startData, Logger& logger, PlayersStatisticsStore& statisticsStore,
         Randomizer& randomizer, HandEvaluationEngine& handEvaluationEngine);

    ~Hand();

    HandActionProcessor* getActionProcessor() const;
    void handlePlayerAction(PlayerAction action); 
    void initialize();
    void runGameLoop();
    void end();
    
    // Card dealing operations (delegated to HandCardDealer)
    void initAndShuffleDeck();
    void dealHoleCards(size_t lastArrayIndex);
    size_t dealBoardCards();

    // Method for round states to deal cards progressively
    std::vector<Card> dealCardsFromDeck(int numCards);

    pkt::core::player::HandCommonContext updateHandCommonContext();

    std::string getStringBoard() const;
    int getPotOdd(const int playerCash, const int playerSet) const;
    float getM(int cash) const;
    int getSmallBlind() const;
    HandState& getState() { return m_stateManager->getCurrentState(); }
    GameState getGameState() const { return m_stateManager->getGameState(); }
    Board& getBoard() { return *m_board; }
    
    // Player management delegation 
    const pkt::core::player::PlayerList getSeatsList() const { return m_playersManager->getSeatsList(); }
    const pkt::core::player::PlayerList getActingPlayersList() const { return m_playersManager->getActingPlayersList(); }
    const pkt::core::player::PlayerList getPlayersInHandList() const { return m_playersManager->getPlayersInHandList(); }
    std::shared_ptr<BettingActions> getBettingActions() const { return m_playersManager->getBettingActions(); }
    HandPlayersManager* getPlayersManager() const { return m_playersManager.get(); }
    int getDealerPlayerId() const { return m_playersManager->getDealerPlayerId(); }
    int getSmallBlindPlayerId() const { return m_playersManager->getSmallBlindPlayerId(); }
    int getBigBlindPlayerId() const { return m_playersManager->getBigBlindPlayerId(); }
    
    // Additional methods needed for lifecycle management
    void filterPlayersWithInsufficientCash() { m_playersManager->filterPlayersWithInsufficientCash(); }

    // Context management methods
    void populateGeneralGameContext(pkt::core::player::HandCommonContext& context);
    void populatePlayersContextInfo(pkt::core::player::HandCommonContext& context);
    void populateBettingContextInfo(pkt::core::player::HandCommonContext& context);

    HandStateManager* getStateManager() const { return m_stateManager.get(); }
    const GameEvents& getEvents() const { return m_events; }
    void fireOnPotUpdated() const;
    pkt::core::player::PlayerList& getActingPlayersListMutable() { return m_playersManager->getActingPlayersListMutable(); }

    const std::vector<pkt::core::BettingRoundHistory>& getHandActionHistory() const
    {
        return getBettingActions()->getHandActionHistory();
    }

  private:
    
    // Constructor helper methods for component initialization
    void initializePlayersManager(const pkt::core::player::PlayerList& seats, 
                                 const pkt::core::player::PlayerList& actingPlayers, 
                                 const StartData& startData);
    void initializeActionHandler();
    void initializeCardAndStateComponents(const StartData& startData);
    void initializeLifecycleManager();
    
    Logger& getLogger() const;
    PlayersStatisticsStore& getPlayersStatisticsStore() const;

    const GameEvents& m_events;
    std::shared_ptr<Board> m_board;

    Logger* m_logger;
    PlayersStatisticsStore* m_statisticsStore;
    Randomizer* m_randomizer;
    HandEvaluationEngine* m_handEvaluationEngine;
    std::unique_ptr<HandStateManager> m_stateManager;
    std::unique_ptr<HandCardDealer> m_cardDealer;
    std::unique_ptr<HandCalculator> m_calculator;
    std::unique_ptr<HandPlayersManager> m_playersManager;
    std::unique_ptr<HandActionHandler> m_actionHandler;
    std::unique_ptr<HandLifecycleManager> m_lifecycleManager;
    int m_startQuantityPlayers;
    int m_startCash;
    int m_smallBlind;

    bool m_allInCondition{false};
    bool m_cardsShown{false};
};

} // namespace pkt::core
