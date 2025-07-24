#pragma once

#include <memory>
#include "core/interfaces/IHand.h"
#include "core/interfaces/hand/IHandLifecycle.h"
#include "core/interfaces/hand/IHandPlayerAction.h"
#include "core/interfaces/hand/IHandState.h"

namespace pkt::core
{

class IHandState;

class HandFsm : public IHand, IHandLifecycle, public IHandPlayerAction, public IHandState
{
  public:
    HandFsm();
    ~HandFsm();

    void startHand() override;
    void endHand() override;

    void handlePlayerAction(int playerId, PlayerAction action) override;

  private:
    std::unique_ptr<IHandState> myState;
};

} // namespace pkt::core
