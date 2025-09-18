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
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDeckDealer.h"
#include "core/interfaces/hand/IHandLifecycle.h"
#include "core/interfaces/hand/IHandPlayerAction.h"
#include "core/interfaces/hand/IHandState.h"
#include "strategy/CurrentHandContext.h"

namespace pkt::core
{

class IHandState;
class EngineFactory;
class IBoard;

class Hand : public IHandLifecycle, public IHandPlayerAction, public HandPlayersState, public IDeckDealer
{
  public:
    Hand(const GameEvents&, std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard>,
         pkt::core::player::PlayerList seats, pkt::core::player::PlayerList actingPlayers, GameData gameData,
         StartData startData);
    ~Hand();

    IActionProcessor* getActionProcessor() const;
    void handlePlayerAction(PlayerAction action) override;
    void initialize() override;
    void runGameLoop() override;
    void end() override;
    size_t dealBoardCards() override;
    void dealHoleCards(size_t lastArrayIndex) override;
    void initAndShuffleDeck() override;

    // Method for round states to deal cards progressively
    std::vector<Card> dealCardsFromDeck(int numCards);

    pkt::core::player::HandCommonContext updateHandCommonContext(const GameState);

    std::string getStringBoard() const;
    int getPotOdd(const int playerCash, const int playerSet) const;
    float getM(int cash) const;
    int getSmallBlind() const;
    IHandState& getState() { return myStateManager->getCurrentState(); }
    GameState getGameState() const { return myStateManager->getGameState(); }
    IBoard& getBoard() { return *myBoard; }

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

    std::shared_ptr<EngineFactory> myFactory;
    const GameEvents& myEvents;
    std::shared_ptr<IBoard> myBoard;
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
