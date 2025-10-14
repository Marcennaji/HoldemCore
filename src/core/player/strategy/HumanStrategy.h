// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <chrono>
#include <functional>
#include <future>
#include "PlayerStrategy.h"
#include "core/engine/GameEvents.h"

namespace pkt::core::player
{

/**
 * @brief Strategy for human players that prompts for manual input.
 *
 * This strategy allows human players to make their own decisions during the game.
 * It handles user interaction through events and provides the interface for
 * manual action selection (fold, call, raise, etc.).
 */
class HumanStrategy : public PlayerStrategy
{
  public:
    HumanStrategy(const pkt::core::GameEvents& events) // Back to const since onProcessEvents is mutable
        : m_events(events)
    {
    }

    pkt::core::PlayerAction decideAction(const CurrentHandContext& ctx) override;
    std::string getName() const override { return "Human"; }
    /**
     * Set the action chosen by the player (called by UI adapters)
     * @param action The action chosen by the player
     */
    void setPlayerAction(const pkt::core::PlayerAction& action);

  private:
    const pkt::core::GameEvents& m_events; // Back to const

    // Promise/future for bridging async UI with synchronous strategy interface
    std::unique_ptr<std::promise<pkt::core::PlayerAction>> m_actionPromise;
};

} // namespace pkt::core::player
