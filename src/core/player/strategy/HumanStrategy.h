#pragma once

#include "PlayerStrategy.h"
#include "core/engine/GameEvents.h"
#include <functional>
#include <future>
#include <chrono>

namespace pkt::core::player
{

class HumanStrategy : public PlayerStrategy
{
  public:
    HumanStrategy(const GameEvents& events)  // Back to const since onProcessEvents is mutable
        : myEvents(events)
    {
    }

    PlayerAction decideAction(const CurrentHandContext& ctx) override;

    /**
     * Set the action chosen by the player (called by UI adapters)
     * @param action The action chosen by the player
     */
    void setPlayerAction(const PlayerAction& action);

  private:
    const GameEvents& myEvents;  // Back to const
    
    // Promise/future for bridging async UI with synchronous strategy interface
    std::unique_ptr<std::promise<PlayerAction>> myActionPromise;
};

} // namespace pkt::core::player
