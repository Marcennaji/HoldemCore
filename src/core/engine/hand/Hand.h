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
#include "core/engine/hand/HandLifecycle.h"
#include "core/engine/hand/HandPlayerAction.h"
#include "core/engine/hand/HandState.h"

#include "strategy/CurrentHandContext.h"

namespace pkt::core
{

class HandState;
class EngineFactory;
class Board;

class Hand : public HandLifecycle
{
  public:

    Hand(const GameEvents&, std::shared_ptr<Board>,
         pkt::core::player::PlayerList seats, pkt::core::player::PlayerList actingPlayers, GameData gameData,
         StartData startData, Logger& logger, PlayersStatisticsStore& statisticsStore,
         Randomizer& randomizer, HandEvaluationEngine& handEvaluationEngine);

    ~Hand();

    HandActionProcessor* getActionProcessor() const;
    void handlePlayerAction(PlayerAction action); 
    void initialize() override;
    void runGameLoop() override;
    void end() override;
    
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
    int getDealerPlayerId() const { return m_playersManager->getDealerPlayerId(); }
    int getSmallBlindPlayerId() const { return m_playersManager->getSmallBlindPlayerId(); }
    int getBigBlindPlayerId() const { return m_playersManager->getBigBlindPlayerId(); }

    HandStateManager* getStateManager() const { return m_stateManager.get(); }
    const GameEvents& getEvents() const { return m_events; }
    void fireOnPotUpdated() const;
    pkt::core::player::PlayerList& getActingPlayersListMutable() { return m_playersManager->getActingPlayersListMutable(); }

    const std::vector<pkt::core::BettingRoundHistory>& getHandActionHistory() const
    {
        return getBettingActions()->getHandActionHistory();
    }

  private:
    
    Logger& getLogger() const;
    PlayersStatisticsStore& getPlayersStatisticsStore() const;
    
    // Cash validation methods (delegates to HandPlayersManager)
    void filterPlayersWithInsufficientCash() { m_playersManager->filterPlayersWithInsufficientCash(); }

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
    int m_startQuantityPlayers;
    int m_startCash;
    int m_smallBlind;

    bool m_allInCondition{false};
    bool m_cardsShown{false};
};

} // namespace pkt::core
