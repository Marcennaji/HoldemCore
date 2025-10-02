
#pragma once

#include <core/services/ServiceContainer.h>
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/interfaces/Logger.h"

#include <map>
#include <memory>
#include <vector>

namespace pkt::core::player
{

class CurrentHandActions
{
  public:
    CurrentHandActions();
    explicit CurrentHandActions(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer);
    ~CurrentHandActions() = default;

    void reset();

    void addAction(const GameState& state, const PlayerAction& action);
    const std::vector<PlayerAction>& getActions(const GameState& state) const { return myActionsByState.at(state); }

    int getActionsNumber(const GameState& state, const ActionType& actionType) const;

    int getLastBetAmount(const GameState& state) const;
    int getRoundTotalBetAmount(const GameState& state) const;
    int getHandTotalBetAmount() const;

    PlayerAction getLastAction() const { return myLastAction; }

    void writeActionsToLog() const;
    void writeActionsToLog(Logger& logger) const;

  private:
    void ensureServicesInitialized() const;
    mutable std::shared_ptr<pkt::core::ServiceContainer> myServices; // Injected service container

    std::map<GameState, std::vector<PlayerAction>> myActionsByState;
    PlayerAction myLastAction{-1, ActionType::None, 0};
};

} // namespace pkt::core::player
