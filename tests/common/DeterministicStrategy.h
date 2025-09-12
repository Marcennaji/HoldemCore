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

// test strategy that will fire the human input event when asked to decide,
// and then provide a deterministic action so the test can continue
class TestHumanEventStrategy : public pkt::test::DeterministicStrategy
{
  private:
    const pkt::core::GameEvents& myEvents;

  public:
    TestHumanEventStrategy(const pkt::core::GameEvents& events) : myEvents(events) {}

    pkt::core::PlayerAction decideAction(const CurrentHandContext& ctx) override
    {
        // Fire the human input event with real valid actions from context
        if (myEvents.onAwaitingHumanInput)
        {
            myEvents.onAwaitingHumanInput(ctx.personalContext.id, ctx.commonContext.validActions);
        }

        // Then return a deterministic action to let the test continue
        return {ctx.personalContext.id, pkt::core::ActionType::Fold};
    }
};
} // namespace pkt::test
