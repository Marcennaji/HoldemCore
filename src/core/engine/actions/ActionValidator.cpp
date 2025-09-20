#include "ActionValidator.h"
#include "BettingActions.h"
#include "Helpers.h"             // For getValidActionsForPlayer
#include "core/player/Helpers.h" // For getPlayerById
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"

#include <algorithm>

namespace pkt::core
{

ActionValidator::ActionValidator(std::shared_ptr<pkt::core::ServiceContainer> services)
    : myServices(std::move(services))
{
}

void ActionValidator::ensureServicesInitialized() const
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

bool ActionValidator::validatePlayerAction(const pkt::core::player::PlayerList& actingPlayersList,
                                           const PlayerAction& action, const BettingActions& bettingActions,
                                           int smallBlind, const GameState gameState) const
{
    ensureServicesInitialized();

    auto player = pkt::core::player::getPlayerById(actingPlayersList, action.playerId);
    if (!player)
    {
        myServices->logger().error(gameStateToString(gameState) + ": player with id " +
                                   std::to_string(action.playerId) + " not found in actingPlayersList");
        return false;
    }

    // Validate consecutive actions
    if (!isConsecutiveActionAllowed(bettingActions, action, gameState))
    {
        return false;
    }

    // Validate action type
    if (!isActionTypeValid(actingPlayersList, action, bettingActions, smallBlind, gameState, player))
    {
        return false;
    }

    // Validate action amount
    if (!isActionAmountValid(action, bettingActions, smallBlind, gameState, player))
    {
        return false;
    }

    return true;
}

// Static helper for backwards compatibility
bool ActionValidator::validate(const pkt::core::player::PlayerList& actingPlayersList, const PlayerAction& action,
                               const BettingActions& bettingActions, int smallBlind, const GameState gameState)
{
    ActionValidator validator;
    return validator.validatePlayerAction(actingPlayersList, action, bettingActions, smallBlind, gameState);
}

bool ActionValidator::isConsecutiveActionAllowed(const BettingActions& bettingActions, const PlayerAction& action,
                                                 const GameState gameState) const
{
    ensureServicesInitialized();

    const auto& handHistory = bettingActions.getHandActionHistory();
    for (const auto& round : handHistory)
    {
        if (round.round == gameState && !round.actions.empty())
        {
            // Get the last action in this round
            const auto& lastAction = round.actions.back();
            if (lastAction.first == action.playerId)
            {
                myServices->logger().error(gameStateToString(gameState) + ": Player " +
                                           std::to_string(action.playerId) +
                                           " cannot act twice consecutively in the same round");
                return false;
            }
            break;
        }
    }
    return true;
}

bool ActionValidator::isActionTypeValid(const pkt::core::player::PlayerList& actingPlayersList,
                                        const PlayerAction& action, const BettingActions& bettingActions,
                                        int smallBlind, const GameState gameState,
                                        const std::shared_ptr<pkt::core::player::Player>& player) const
{
    ensureServicesInitialized();

    std::vector<ActionType> validActions =
        getValidActionsForPlayer(actingPlayersList, action.playerId, bettingActions, smallBlind, gameState);

    bool isValid = std::find(validActions.begin(), validActions.end(), action.type) != validActions.end();

    if (!isValid)
    {
        myServices->logger().error(gameStateToString(gameState) + ": Invalid action type for player " +
                                   player->getName() + " : " + actionTypeToString(action.type));
    }

    return isValid;
}

bool ActionValidator::isActionAmountValid(const PlayerAction& action, const BettingActions& bettingActions,
                                          int smallBlind, const GameState gameState,
                                          const std::shared_ptr<pkt::core::player::Player>& player) const
{
    ensureServicesInitialized();

    const int currentHighestBet = bettingActions.getRoundHighestSet();
    const int playerBet = player->getCurrentHandActions().getRoundTotalBetAmount(gameState);

    bool isValid = true;

    switch (action.type)
    {
    case ActionType::Fold:
        // Fold doesn't require amount validation
        break;

    case ActionType::Check:
        isValid = (action.amount == 0);
        break;

    case ActionType::Call:
        // Amount will be calculated by the system
        break;

    case ActionType::Bet:
        isValid = (action.amount > 0 && action.amount <= player->getCash());
        break;

    case ActionType::Raise:
    {
        isValid = (action.amount > currentHighestBet);

        int minRaise = bettingActions.getMinRaise(smallBlind);
        isValid = (isValid && action.amount >= currentHighestBet + minRaise);

        const int extraChipsRequired = action.amount - playerBet;
        isValid = (isValid && extraChipsRequired <= player->getCash());
        break;
    }
    case ActionType::Allin:
        // All-in doesn't require specific amount validation
        break;

    default:
        isValid = false;
    }

    if (!isValid)
    {
        myServices->logger().error(gameStateToString(gameState) + ": Invalid action amount for player " +
                                   std::to_string(action.playerId) + " : " + actionTypeToString(action.type) +
                                   " with amount = " + std::to_string(action.amount));
    }

    return isValid;
}

} // namespace pkt::core