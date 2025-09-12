#pragma once

#include "PlayerStrategy.h"
#include "core/engine/GameEvents.h"
#include "core/ui/UserInputHandler.h"

namespace pkt::core::player
{

class HumanStrategy : public PlayerStrategy
{
  public:
    HumanStrategy(pkt::core::ui::UserInputHandler& inputHandler, const pkt::core::GameEvents& events)
        : myInputHandler(inputHandler), myEvents(events)
    {
    }

    PlayerAction decideAction(const CurrentHandContext& ctx) override;

  private:
    pkt::core::ui::UserInputHandler& myInputHandler;
    const pkt::core::GameEvents& myEvents;
};

} // namespace pkt::core::player
