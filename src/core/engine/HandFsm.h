#pragma once

#include <memory>
#include "PlayerFsm.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/interfaces/hand/IDeckDealer.h"
#include "core/interfaces/hand/IHandLifecycle.h"
#include "core/interfaces/hand/IHandPlayerAction.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/interfaces/hand/IPlayerAccess.h"
namespace pkt::core
{

class IHandState;
class EngineFactory;
class IBoard;

class HandFsm : public IHandLifecycle, public IHandPlayerAction, public IPlayerAccess, public IDeckDealer
{
  public:
    HandFsm(const GameEvents&, std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard>,
            pkt::core::player::PlayerFsmList seats, pkt::core::player::PlayerFsmList runningPlayers, GameData gameData,
            StartData startData);
    ~HandFsm();

    void start() override;
    void end() override;
    size_t dealBoardCards() override;
    void dealHoleCards(size_t lastArrayIndex) override;
    void initAndShuffleDeck() override;

    void processPlayerAction(PlayerAction action) override;
    pkt::core::player::PlayerFsmList getSeatsList() const override;
    pkt::core::player::PlayerFsmList getRunningPlayersList() const override;

  private:
    std::shared_ptr<EngineFactory> myFactory;
    const GameEvents& myEvents;
    std::shared_ptr<IBoard> myBoard;
    std::unique_ptr<IHandState> myState;
    pkt::core::player::PlayerFsmList mySeatsList;          // all players
    pkt::core::player::PlayerFsmList myRunningPlayersList; // all players who have not folded and are not all in
    std::vector<int> myCardsArray;
    int myStartQuantityPlayers;
    int myStartCash;
    unsigned myDealerPlayerId;
    unsigned mySmallBlindPlayerId;
    unsigned myBigBlindPlayerId;
    int mySmallBlind;

    int myPreviousPlayerId{-1};
    int myPreflopLastRaiserId;
    int myFlopLastRaiserId;
    int myTurnLastRaiserId;
    unsigned myLastActionPlayerId{0};

    bool myAllInCondition{false};
    bool myCardsShown{false};
};

} // namespace pkt::core
