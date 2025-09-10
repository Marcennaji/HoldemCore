#pragma once

#include <memory>
#include "PlayerFsm.h"
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

    pkt::core::player::HandCommonContext updateHandCommonContext(const GameState);

    std::string getStringBoard() const;
    int getPotOdd(const int playerCash, const int playerSet) const;
    float getM(int cash) const;
    int getSmallBlind() const;
    IHandState& getState() { return *myState; }
    GameState getGameState() const { return myState->getGameState(); }
    IBoard& getBoard() { return *myBoard; }

  private:
    void applyActionEffects(const PlayerAction action);

    std::shared_ptr<EngineFactory> myFactory;
    const GameEvents& myEvents;
    std::shared_ptr<IBoard> myBoard;
    std::unique_ptr<IHandState> myState;
    std::vector<int> myCardsArray;
    int myStartQuantityPlayers;
    int myStartCash;
    int mySmallBlind;

    bool myAllInCondition{false};
    bool myCardsShown{false};
};

} // namespace pkt::core
