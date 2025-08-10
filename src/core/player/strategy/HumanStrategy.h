#pragma once

#include "PlayerStrategy.h"
#include "core/ui/UserInputHandler.h"

namespace pkt::core::player
{

class HumanStrategy : public PlayerStrategy
{
  public:
    HumanStrategy(pkt::core::ui::UserInputHandler& inputHandler) : myInputHandler(inputHandler) {}

    PlayerAction decideAction(const CurrentHandContext& ctx) override { return myInputHandler.requestAction(ctx); }

  private:
    pkt::core::ui::UserInputHandler& myInputHandler;
};

} // namespace pkt::core::player
