#pragma once

#include "core/engine/model/PlayerAction.h"
#include "core/player/strategy/BotStrategy.h"
#include "core/player/strategy/CurrentHandContext.h"

namespace pkt::test
{
using CurrentHandContext = pkt::core::player::CurrentHandContext;

class DeterministicStrategy : public pkt::core::player::BotStrategy
{
  public:
    void setLastAction(pkt::core::GameState state, pkt::core::PlayerAction action) { myActions[state] = action; }

    void clear() { myActions.clear(); }

  private:
    pkt::core::PlayerAction decidePreflop(const CurrentHandContext& ctx) override
    {
        return decideForState(ctx, pkt::core::GameState::Preflop);
    }
    pkt::core::PlayerAction decideFlop(const CurrentHandContext& ctx) override
    {
        return decideForState(ctx, pkt::core::GameState::Flop);
    }
    pkt::core::PlayerAction decideTurn(const CurrentHandContext& ctx) override
    {
        return decideForState(ctx, pkt::core::GameState::Turn);
    }
    pkt::core::PlayerAction decideRiver(const CurrentHandContext& ctx) override
    {
        return decideForState(ctx, pkt::core::GameState::River);
    }

    pkt::core::PlayerAction decideForState(const CurrentHandContext& ctx, pkt::core::GameState state)
    {
        auto it = myActions.find(state);
        if (it != myActions.end())
        {
            return it->second;
        }
        return {ctx.personalContext.id, pkt::core::ActionType::None, 0};
    }

    std::unordered_map<pkt::core::GameState, pkt::core::PlayerAction> myActions;
};
} // namespace pkt::test
