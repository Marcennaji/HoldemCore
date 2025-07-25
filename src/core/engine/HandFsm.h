#pragma once

#include <memory>
#include "core/interfaces/IHand.h"
#include "core/interfaces/hand/IHandLifecycle.h"
#include "core/interfaces/hand/IHandPlayerAction.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/interfaces/hand/IPlayerAccess.h"

namespace pkt::core
{

class IHandState;

class HandFsm : public IHandLifecycle, public IHandPlayerAction, public IHandState, public IPlayerAccess
{
  public:
    HandFsm();
    ~HandFsm();

    void startHand() override;
    void endHand() override;

    void handlePlayerAction(int playerId, PlayerAction action) override;

  private:
    std::unique_ptr<IHandState> myState;
    pkt::core::player::PlayerList mySeatsList;          // all players
    pkt::core::player::PlayerList myRunningPlayersList; // all players who have not folded and are not all in
};

} // namespace pkt::core
