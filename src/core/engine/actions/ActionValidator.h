#pragma once

#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"
#include "typedefs.h"

#include <memory>
#include <string>

namespace pkt::core
{
class ServiceContainer;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;
} // namespace pkt::core::player

namespace pkt::core
{

class BettingActions;

/**
 * @brief ActionValidator handles comprehensive validation of player actions
 *
 * This class migrates validation logic from Helpers.cpp to provide centralized
 * action validation with proper error reporting. It validates action types,
 * amounts, consecutive actions, and game rules.
 */
class ActionValidator
{
  public:
    ActionValidator() = default;
    explicit ActionValidator(std::shared_ptr<pkt::core::ServiceContainer> services);
    ~ActionValidator() = default;

    // Main validation method - migrated from Helpers.cpp
    bool validatePlayerAction(const pkt::core::player::PlayerList& actingPlayersList, const PlayerAction& action,
                              const BettingActions& bettingActions, int smallBlind, const GameState gameState) const;

    // Static helper for backwards compatibility with existing code
    static bool validate(const pkt::core::player::PlayerList& actingPlayersList, const PlayerAction& action,
                         const BettingActions& bettingActions, int smallBlind, const GameState gameState);

    // Validation helper methods - migrated from Helpers.cpp
    bool isConsecutiveActionAllowed(const BettingActions& bettingActions, const PlayerAction& action,
                                    const GameState gameState) const;

    bool isActionTypeValid(const pkt::core::player::PlayerList& actingPlayersList, const PlayerAction& action,
                           const BettingActions& bettingActions, int smallBlind, const GameState gameState,
                           const std::shared_ptr<pkt::core::player::Player>& player) const;

    bool isActionAmountValid(const PlayerAction& action, const BettingActions& bettingActions, int smallBlind,
                             const GameState gameState, const std::shared_ptr<pkt::core::player::Player>& player) const;

  private:
    mutable std::shared_ptr<pkt::core::ServiceContainer> myServices;
    void ensureServicesInitialized() const;
};

} // namespace pkt::core