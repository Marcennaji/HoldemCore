#include "ActionValidator.h"
#include "BettingActions.h"
#include "Helpers.h"             // For getValidActionsForPlayer
#include "core/player/Helpers.h" // For getPlayerById
#include "core/player/Player.h"
#include "core/interfaces/NullLogger.h"

#include <algorithm>

namespace pkt::core
{

ActionValidator::ActionValidator(std::shared_ptr<pkt::core::Logger> logger)
    : m_logger(logger)
{
}

pkt::core::Logger& ActionValidator::getLogger() const
{
    if (m_logger) {
        return *m_logger;
    }
    static pkt::core::NullLogger nullLogger;
    return nullLogger;
}

bool ActionValidator::validatePlayerAction(const pkt::core::player::PlayerList& actingPlayersList,
                                           const PlayerAction& action, const BettingActions& bettingActions,
                                           int smallBlind, const GameState gameState) const
{
    auto player = pkt::core::player::getPlayerById(actingPlayersList, action.playerId);
    if (!player)
    {
        getLogger().error(gameStateToString(gameState) + ": player with id " +
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

bool ActionValidator::validatePlayerActionWithReason(const pkt::core::player::PlayerList& actingPlayersList,
                                                    const PlayerAction& action, const BettingActions& bettingActions,
                                                    int smallBlind, const GameState gameState,
                                                    std::string& outReason) const
{
    auto player = pkt::core::player::getPlayerById(actingPlayersList, action.playerId);
    if (!player)
    {
        outReason = "Player not found in active players list";
        getLogger().error(gameStateToString(gameState) + ": player with id " +
                                   std::to_string(action.playerId) + " not found in actingPlayersList");
        return false;
    }

    // Validate consecutive actions
    if (!isConsecutiveActionAllowed(bettingActions, action, gameState, &outReason))
    {
        return false;
    }

    // Validate action type
    if (!isActionTypeValid(actingPlayersList, action, bettingActions, smallBlind, gameState, player, &outReason))
    {
        return false;
    }

    // Validate action amount
    if (!isActionAmountValid(action, bettingActions, smallBlind, gameState, player, &outReason))
    {
        return false;
    }

    outReason.clear();
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
                                                 const GameState gameState, std::string* outReason) const
{
    const auto& handHistory = bettingActions.getHandActionHistory();
    for (const auto& round : handHistory)
    {
        if (round.round == gameState && !round.actions.empty())
        {
            // Find the last voluntary action (exclude blind posts)
            std::pair<unsigned, ActionType> lastVoluntary = {static_cast<unsigned>(-1), ActionType::None};
            for (auto it = round.actions.rbegin(); it != round.actions.rend(); ++it)
            {
                if (it->second == ActionType::PostSmallBlind || it->second == ActionType::PostBigBlind)
                    continue; // ignore automatic blind posts
                lastVoluntary = *it;
                break;
            }

            // If the last voluntary action was by the same player, reject consecutive action
            if (lastVoluntary.second != ActionType::None && lastVoluntary.first == static_cast<unsigned>(action.playerId))
            {
                std::string msg = "Cannot act twice consecutively in the same round: last voluntary was " +
                                  std::string(actionTypeToString(lastVoluntary.second)) + " by player " +
                                  std::to_string(action.playerId);
                if (outReason) *outReason = msg;
                getLogger().error(gameStateToString(gameState) + ": " + msg);
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
                                        const std::shared_ptr<pkt::core::player::Player>& player,
                                        std::string* outReason) const
{
    std::vector<ActionType> validActions =
        getValidActionsForPlayer(actingPlayersList, action.playerId, bettingActions, smallBlind, gameState);

    // Trace valid actions for debugging
    {
        std::string actionsStr;
        for (size_t i = 0; i < validActions.size(); ++i)
        {
            actionsStr += actionTypeToString(validActions[i]);
            if (i + 1 < validActions.size()) actionsStr += ",";
        }
        getLogger().debug(gameStateToString(gameState) + 
                                   ": valid actions for player " + player->getName() + 
                                   " => [" + actionsStr + "] (requested: " + actionTypeToString(action.type) + ")");
    }

    bool isValid = std::find(validActions.begin(), validActions.end(), action.type) != validActions.end();

    // Fallback: If a player attempts to Call and helpers didn't include it, allow Call when there's
    // an outstanding bet to match and the player still has chips. This covers edge cases around
    // all-in sizing and highest bet tracking.
    if (!isValid && action.type == ActionType::Call)
    {
        const int currentHighestBet = bettingActions.getRoundHighestSet();
        const int playerBet = player->getCurrentHandActions().getRoundTotalBetAmount(gameState);
        const int playerCash = player->getCash();
        if (playerBet < currentHighestBet && playerCash > 0)
        {
            isValid = true;
        }
        else
        {
            // Additional safety: if an All-in occurred this round and player still has chips, allow a Call
            const auto& history = bettingActions.getHandActionHistory();
            auto roundIt = std::find_if(history.begin(), history.end(),
                                        [gameState](const BettingRoundHistory& h) { return h.round == gameState; });
            if (roundIt != history.end())
            {
                const bool anyAllin = std::any_of(roundIt->actions.begin(), roundIt->actions.end(),
                                                  [](const std::pair<unsigned, ActionType>& p)
                                                  { return p.second == ActionType::Allin; });
                if (anyAllin && playerCash > 0)
                {
                    isValid = true;
                }
            }
        }
    }

    if (!isValid)
    {
    std::string msg = std::string("Invalid action type: ") + actionTypeToString(action.type);
        if (outReason) *outReason = msg;
        getLogger().error(gameStateToString(gameState) + ": Invalid action type for player " +
                                   player->getName() + " : " + actionTypeToString(action.type));
    }

    return isValid;
}

bool ActionValidator::isActionAmountValid(const PlayerAction& action, const BettingActions& bettingActions,
                                          int smallBlind, const GameState gameState,
                                          const std::shared_ptr<pkt::core::player::Player>& player,
                                          std::string* outReason) const
{

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
    std::string msg = std::string("Invalid amount for ") + actionTypeToString(action.type) +
                          ": amount=" + std::to_string(action.amount);
        switch (action.type)
        {
        case ActionType::Check:
            msg += ", expected amount=0";
            break;
        case ActionType::Raise:
        {
            const int currentHighestBet = bettingActions.getRoundHighestSet();
            int minRaise = bettingActions.getMinRaise(smallBlind);
            msg += ": It needs to be >= " + std::to_string(currentHighestBet + minRaise) + ", and within player's cash";
            break;
        }
        default:
            break;
        }
        if (outReason) *outReason = msg;
        getLogger().error(gameStateToString(gameState) + ": Invalid action amount for player " +
                                   std::to_string(action.playerId) + " : " + actionTypeToString(action.type) +
                                   " with amount = " + std::to_string(action.amount));
    }

    return isValid;
}

} // namespace pkt::core