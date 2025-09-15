#pragma once

#include <map>
#include <memory>
#include "PlayerFsm.h"
#include "core/cards/Card.h"
#include "core/engine/HandPlayersState.h"
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

class HandFsm : public IHandLifecycle, public IHandPlayerAction, public HandPlayersState, public IDeckDealer
{
  public:
    HandFsm(const GameEvents&, std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard>,
            pkt::core::player::PlayerFsmList seats, pkt::core::player::PlayerFsmList actingPlayers, GameData gameData,
            StartData startData);
    ~HandFsm();

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
    IHandState& getState() { return *myState; }
    GameState getGameState() const { return myState->getGameState(); }
    IBoard& getBoard() { return *myBoard; }

    // Hand action history methods (delegates to BettingActions)
    const std::vector<pkt::core::BettingRoundHistory>& getHandActionHistory() const
    {
        return getBettingActions()->getHandActionHistory();
    }

  private:
    void applyActionEffects(const PlayerAction action);
    void fireOnPotUpdated() const;
    std::string getActionValidationError(const PlayerAction& action) const;
    PlayerAction getDefaultActionForPlayer(unsigned playerId) const;
    void resetInvalidActionCount(unsigned playerId);
    bool shouldAutoFoldPlayer(unsigned playerId) const;

    // New focused methods for handlePlayerAction refactoring
    void validateGameState() const;
    IActionProcessor* getActionProcessorOrThrow() const;
    void handleInvalidAction(const PlayerAction& action);
    void handleAutoFold(unsigned playerId);
    void processValidAction(const PlayerAction& action);

    std::shared_ptr<EngineFactory> myFactory;
    const GameEvents& myEvents;
    std::shared_ptr<IBoard> myBoard;
    std::unique_ptr<IHandState> myState;
    Deck myDeck;
    int myStartQuantityPlayers;
    int myStartCash;
    int mySmallBlind;

    // Error handling state
    std::map<unsigned, int> myInvalidActionCounts;    // Track invalid actions per player
    static const int MAX_INVALID_ACTIONS = 3;         // Max invalid actions before auto-fold
    static const int MAX_GAME_LOOP_ITERATIONS = 1000; // Emergency brake for infinite loops

    bool myAllInCondition{false};
    bool myCardsShown{false};
};

} // namespace pkt::core
