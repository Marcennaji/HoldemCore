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
    HumanStrategy(const pkt::core::GameEvents& events)  // Back to const since onProcessEvents is mutable
        : m_events(events)
    {
    }

    pkt::core::PlayerAction decideAction(const CurrentHandContext& ctx) override;

    /**
     * Set the action chosen by the player (called by UI adapters)
     * @param action The action chosen by the player
     */
    void setPlayerAction(const pkt::core::PlayerAction& action);

  private:
    const pkt::core::GameEvents& m_events;  // Back to const

    // Promise/future for bridging async UI with synchronous strategy interface
    std::unique_ptr<std::promise<pkt::core::PlayerAction>> m_actionPromise;
};

} // namespace pkt::core::player
