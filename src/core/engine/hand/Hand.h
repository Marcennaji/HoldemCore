#pragma once

#include <map>
#include <memory>
#include "../actions/ActionValidator.h"
#include "../actions/InvalidActionHandler.h"
#include "../cards/Card.h"
#include "Player.h"
#include "core/engine/cards/DeckManager.h"
#include "core/engine/hand/HandPlayersState.h"
#include "core/engine/hand/HandStateManager.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/engine/hand/ActionProcessor.h"
#include "core/engine/hand/DeckDealer.h"
#include "core/engine/hand/HandLifecycle.h"
#include "core/engine/hand/HandPlayerAction.h"
#include "core/engine/hand/HandState.h"

#include "strategy/CurrentHandContext.h"

namespace pkt::core
{

class HandState;
class EngineFactory;
class Board;

class Hand : public HandLifecycle, public HandPlayerAction, public HandPlayersState, public DeckDealer
{
  public:

    Hand(const GameEvents&, std::shared_ptr<Board>,
         pkt::core::player::PlayerList seats, pkt::core::player::PlayerList actingPlayers, GameData gameData,
         StartData startData, Logger& logger, PlayersStatisticsStore& statisticsStore,
         Randomizer& randomizer, HandEvaluationEngine& handEvaluationEngine);

    ~Hand();

    HandActionProcessor* getActionProcessor() const;
    void handlePlayerAction(PlayerAction action) override;
    void initialize() override;
    void runGameLoop() override;
    void end() override;
    size_t dealBoardCards() override;
    void dealHoleCards(size_t lastArrayIndex) override;
    void initAndShuffleDeck() override;

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

    HandStateManager* getStateManager() const { return m_stateManager.get(); }
    const GameEvents& getEvents() const { return m_events; }
    void fireOnPotUpdated() const;
    pkt::core::player::PlayerList& getActingPlayersListMutable() { return m_actingPlayersList; }

    // Hand action history methods (delegates to BettingActions)
    const std::vector<pkt::core::BettingRoundHistory>& getHandActionHistory() const
    {
        return getBettingActions()->getHandActionHistory();
    }

  private:
    std::string getActionValidationError(const PlayerAction& action) const;
    PlayerAction getDefaultActionForPlayer(unsigned playerId) const;

    // New focused methods for handlePlayerAction refactoring
    void handleAutoFold(unsigned playerId);
    void processValidAction(const PlayerAction& action);
    
    Logger& getLogger() const;
    PlayersStatisticsStore& getPlayersStatisticsStore() const;
    
    // Cash validation methods
    void filterPlayersWithInsufficientCash();

    const GameEvents& m_events;
    std::shared_ptr<Board> m_board;

    Logger* m_logger;
    PlayersStatisticsStore* m_statisticsStore;
    Randomizer* m_randomizer;
    HandEvaluationEngine* m_handEvaluationEngine;
    std::unique_ptr<HandStateManager> m_stateManager;
    std::unique_ptr<DeckManager> m_deckManager;
    std::unique_ptr<ActionValidator> m_actionValidator;
    std::unique_ptr<InvalidActionHandler> m_invalidActionHandler;
    int m_startQuantityPlayers;
    int m_startCash;
    int m_smallBlind;

    bool m_allInCondition{false};
    bool m_cardsShown{false};
};

} // namespace pkt::core
