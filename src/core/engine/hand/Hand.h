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
#include "core/services/PokerServices.h"
#include "strategy/CurrentHandContext.h"

namespace pkt::core
{

class HandState;
class EngineFactory;
class Board;

class Hand : public HandLifecycle, public HandPlayerAction, public HandPlayersState, public DeckDealer
{
  public:
    Hand(const GameEvents&, std::shared_ptr<EngineFactory> f, std::shared_ptr<Board>,
         pkt::core::player::PlayerList seats, pkt::core::player::PlayerList actingPlayers, GameData gameData,
         StartData startData);

    // Constructor with PokerServices for dependency injection
    Hand(const GameEvents&, std::shared_ptr<EngineFactory> f, std::shared_ptr<Board>,
         pkt::core::player::PlayerList seats, pkt::core::player::PlayerList actingPlayers, GameData gameData,
         StartData startData, std::shared_ptr<PokerServices> services);

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
    HandState& getState() { return myStateManager->getCurrentState(); }
    GameState getGameState() const { return myStateManager->getGameState(); }
    Board& getBoard() { return *myBoard; }

    // Accessor methods for ActionApplier
    HandStateManager* getStateManager() const { return myStateManager.get(); }
    const GameEvents& getEvents() const { return myEvents; }
    void fireOnPotUpdated() const;
    pkt::core::player::PlayerList& getActingPlayersListMutable() { return myActingPlayersList; }

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
    void ensureServicesInitialized() const;
    
    // Cash validation methods
    void filterPlayersWithInsufficientCash();

    std::shared_ptr<EngineFactory> myFactory;
    const GameEvents& myEvents;
    std::shared_ptr<Board> myBoard;
    mutable std::shared_ptr<PokerServices> myServices; // Injected service container
    std::unique_ptr<HandStateManager> myStateManager;
    std::unique_ptr<DeckManager> myDeckManager;
    std::unique_ptr<ActionValidator> myActionValidator;
    std::unique_ptr<InvalidActionHandler> myInvalidActionHandler;
    int myStartQuantityPlayers;
    int myStartCash;
    int mySmallBlind;

    bool myAllInCondition{false};
    bool myCardsShown{false};
};

} // namespace pkt::core
