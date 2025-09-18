
#include "Helpers.h"
#include "core/engine/BettingActions.h"
#include "core/engine/CardUtilities.h"
#include "core/engine/Exception.h"
#include "core/engine/Hand.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/services/GlobalServices.h"
// Include FSM states for the helper function
#include "core/engine/round_states/FlopState.h"
#include "core/engine/round_states/PostRiverState.h"
#include "core/engine/round_states/RiverState.h"
#include "core/engine/round_states/TurnState.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

static std::vector<ActionType> getValidActionsForPlayer(const PlayerList& actingPlayersList, int playerId,
                                                        const BettingActions& bettingActions, int smallBlind,
                                                        const GameState gameState);

std::vector<ActionType> getValidActionsForPlayer(const Hand& hand, int playerId)
{
    int smallBlind = hand.getSmallBlind();

    return getValidActionsForPlayer(hand.getActingPlayersList(), playerId, *hand.getBettingActions(), smallBlind,
                                    hand.getGameState());
}

std::vector<ActionType> getValidActionsForPlayer(const PlayerList& actingPlayersList, int playerId,
                                                 const BettingActions& bettingActions, int smallBlind,
                                                 const GameState gameState)
{
    std::vector<ActionType> validActions;

    // Find the player in the acting players list
    auto player = getPlayerById(actingPlayersList, playerId);
    if (!player)
    {
        return validActions; // Return empty list if player not found
    }

    const int currentHighestBet = bettingActions.getRoundHighestSet();
    const int playerBet = player->getCurrentHandActions().getRoundTotalBetAmount(gameState);
    const int playerCash = player->getCash();

    // Fold is almost always available (except in some edge cases like being all-in)
    if (playerCash > 0 || playerBet < currentHighestBet)
    {
        validActions.push_back(ActionType::Fold);
    }

    // Check if player can check (when no bet to call)
    if (playerBet == currentHighestBet)
    {
        validActions.push_back(ActionType::Check);
    }

    // Check if player can call (when there's a bet to call and player has chips)
    if (playerBet < currentHighestBet && playerCash > 0)
    {
        validActions.push_back(ActionType::Call);
    }

    // Check if player can bet (when no current bet and player has chips)
    if (currentHighestBet == 0 && playerCash > 0)
    {
        validActions.push_back(ActionType::Bet);
    }

    // Check if player can raise (when there's a current bet and player has enough chips)
    if (currentHighestBet > 0 && playerCash > 0)
    {
        // Get minimum raise amount based on game rules
        int minRaise = bettingActions.getMinRaise(smallBlind);
        int minRaiseAmount = currentHighestBet + minRaise;
        int extraChipsRequired = minRaiseAmount - playerBet;

        if (extraChipsRequired <= playerCash)
        {
            validActions.push_back(ActionType::Raise);
        }
    }

    // All-in is available if player has chips
    if (playerCash > 0)
    {
        validActions.push_back(ActionType::Allin);
    }

    return validActions;
}

std::shared_ptr<player::Player> getFirstPlayerToActPostFlop(const Hand& hand)
{
    auto actingPlayers = hand.getActingPlayersList();

    if (actingPlayers->empty())
        return nullptr;

    for (auto it = actingPlayers->begin(); it != actingPlayers->end(); ++it)
    {
        if ((*it)->getPosition() == PlayerPosition::Button || (*it)->getPosition() == PlayerPosition::ButtonSmallBlind)
        {
            auto nextIt = std::next(it);
            if (nextIt == actingPlayers->end())
                nextIt = actingPlayers->begin();

            return *nextIt;
        }
    }

    return actingPlayers->front();
}

std::unique_ptr<pkt::core::IHandState> computeBettingRoundNextState(pkt::core::Hand& hand,
                                                                    const pkt::core::GameEvents& events,
                                                                    pkt::core::GameState currentState)
{
    // If less than 2 players are still in hand (haven't folded), go directly to showdown
    if (hand.getPlayersInHandList()->size() < 2)
    {
        return std::make_unique<pkt::core::PostRiverState>(events);
    }

    // If all remaining players are all-in (no one can act further), go directly to showdown
    if (hand.getActingPlayersList()->empty() && hand.getPlayersInHandList()->size() >= 1)
    {
        return std::make_unique<pkt::core::PostRiverState>(events);
    }

    // If round is complete, check if we can continue betting
    if (isRoundComplete(hand))
    {
        // If only one or no players can still act, go directly to showdown
        if (hand.getActingPlayersList()->size() <= 1)
        {
            return std::make_unique<pkt::core::PostRiverState>(events);
        }

        // Multiple players can still act, determine next state based on current state
        switch (currentState)
        {
        case pkt::core::GameState::Preflop:
            return std::make_unique<pkt::core::FlopState>(events);
        case pkt::core::GameState::Flop:
            return std::make_unique<pkt::core::TurnState>(events);
        case pkt::core::GameState::Turn:
            return std::make_unique<pkt::core::RiverState>(events);
        case pkt::core::GameState::River:
            return std::make_unique<pkt::core::PostRiverState>(events);
        default:
            // Shouldn't happen for betting rounds
            return std::make_unique<pkt::core::PostRiverState>(events);
        }
    }

    return nullptr; // Stay in current state - more betting needed
}

bool isRoundComplete(const Hand& hand)
{
    assert(hand.getGameState() != GameState::None);

    for (auto player = hand.getActingPlayersList()->begin(); player != hand.getActingPlayersList()->end(); ++player)
    {
        GlobalServices::instance().logger().verbose("checking if round " + gameStateToString(hand.getGameState()) +
                                                    " is complete : Checking player: " + (*player)->getName());

        if ((*player)->getLastAction().type == ActionType::None ||
            (*player)->getLastAction().type == ActionType::PostBigBlind ||
            (*player)->getLastAction().type == ActionType::PostSmallBlind)
        {
            GlobalServices::instance().logger().verbose("  ROUND NOT COMPLETE, as player " + (*player)->getName() +
                                                        " did not act.");
            return false;
        }

        GlobalServices::instance().logger().verbose(
            "  player round bet amount: " +
            std::to_string((*player)->getCurrentHandActions().getRoundTotalBetAmount(hand.getGameState())) +
            ", hand total bet amount : " + std::to_string((*player)->getCurrentHandActions().getHandTotalBetAmount()) +
            " vs current round highest bet: " + std::to_string(hand.getBettingActions()->getRoundHighestSet()));

        if ((*player)->getCurrentHandActions().getRoundTotalBetAmount(hand.getGameState()) <
            hand.getBettingActions()->getRoundHighestSet())
        {
            GlobalServices::instance().logger().verbose("  ROUND NOT COMPLETE, as player " + (*player)->getName() +
                                                        " has not matched the highest bet yet.");
            return false;
        }
    }
    GlobalServices::instance().logger().verbose("  ROUND " + gameStateToString(hand.getGameState()) + " COMPLETE");
    return true;
}

// Helper function to check for consecutive actions by the same player
bool isConsecutiveActionAllowed(const BettingActions& bettingActions, const PlayerAction& action,
                                const GameState gameState)
{
    const auto& handHistory = bettingActions.getHandActionHistory();
    for (const auto& round : handHistory)
    {
        if (round.round == gameState && !round.actions.empty())
        {
            // Get the last action in this round
            const auto& lastAction = round.actions.back();
            if (lastAction.first == action.playerId)
            {
                GlobalServices::instance().logger().error(gameStateToString(gameState) + ": Player " +
                                                          std::to_string(action.playerId) +
                                                          " cannot act twice consecutively in the same round");
                return false;
            }
            break;
        }
    }
    return true;
}

// Helper function to check if action type is valid for the player
bool isActionTypeValid(const PlayerList& actingPlayersList, const PlayerAction& action,
                       const BettingActions& bettingActions, int smallBlind, const GameState gameState,
                       const std::shared_ptr<player::Player>& player)
{
    std::vector<ActionType> validActions =
        getValidActionsForPlayer(actingPlayersList, action.playerId, bettingActions, smallBlind, gameState);

    bool isValid = std::find(validActions.begin(), validActions.end(), action.type) != validActions.end();

    if (!isValid)
    {
        GlobalServices::instance().logger().error(gameStateToString(gameState) + ": Invalid action type for player " +
                                                  player->getName() + " : " + actionTypeToString(action.type));
    }

    return isValid;
}

// Helper function to validate action amounts
bool isActionAmountValid(const PlayerAction& action, const BettingActions& bettingActions, int smallBlind,
                         const GameState gameState, const std::shared_ptr<player::Player>& player)
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
        GlobalServices::instance().logger().error(
            gameStateToString(gameState) + ": Invalid action amount for player " + std::to_string(action.playerId) +
            " : " + actionTypeToString(action.type) + " with amount = " + std::to_string(action.amount));
    }

    return isValid;
}

bool validatePlayerAction(const PlayerList& actingPlayersList, const PlayerAction& action,
                          const BettingActions& bettingActions, int smallBlind, const GameState gameState)
{
    auto player = pkt::core::player::getPlayerById(actingPlayersList, action.playerId);
    if (!player)
    {
        GlobalServices::instance().logger().error(gameStateToString(gameState) + ": player with id " +
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

std::shared_ptr<player::Player> getNextPlayerToActInRound(const Hand& hand, GameState currentRound)
{
    auto actingPlayers = hand.getActingPlayersList();
    if (actingPlayers->empty())
        return nullptr;

    // Get the hand action history to see what actions have happened
    const auto& handHistory = hand.getBettingActions()->getHandActionHistory();

    // Find the current round and check if there are any actions
    for (const auto& roundHistory : handHistory)
    {
        if (roundHistory.round == currentRound && !roundHistory.actions.empty())
        {
            // For Preflop, skip blind actions to find the last non-blind action
            if (currentRound == GameState::Preflop)
            {
                // Look for the last non-blind action
                for (auto it = roundHistory.actions.rbegin(); it != roundHistory.actions.rend(); ++it)
                {
                    const auto& [playerId, action] = *it;

                    // Skip blind actions
                    if (action != ActionType::PostSmallBlind && action != ActionType::PostBigBlind)
                    {
                        // Found the last non-blind action, find the next player
                        for (auto playerIt = actingPlayers->begin(); playerIt != actingPlayers->end(); ++playerIt)
                        {
                            if ((*playerIt)->getId() == playerId)
                            {
                                // Found the player who just acted, return the next player in sequence
                                auto nextIt = std::next(playerIt);
                                if (nextIt == actingPlayers->end())
                                    nextIt = actingPlayers->begin(); // Wrap around

                                return *nextIt;
                            }
                        }
                    }
                }
            }
            else
            {
                // For postflop rounds, use the last action directly
                const auto& [playerId, action] = roundHistory.actions.back();

                // Found the last action, find the next player
                for (auto playerIt = actingPlayers->begin(); playerIt != actingPlayers->end(); ++playerIt)
                {
                    if ((*playerIt)->getId() == playerId)
                    {
                        // Found the player who just acted, return the next player in sequence
                        auto nextIt = std::next(playerIt);
                        if (nextIt == actingPlayers->end())
                            nextIt = actingPlayers->begin(); // Wrap around

                        return *nextIt;
                    }
                }
            }
        }
    }

    // No actions found for this round, this is the start of the round
    if (currentRound == GameState::Preflop)
    {
        // Preflop: start from left of big blind (first non-blind action)
        for (auto it = actingPlayers->begin(); it != actingPlayers->end(); ++it)
        {
            if ((*it)->getPosition() == PlayerPosition::BigBlind)
            {
                // Found the big blind, get the next player in the list
                auto nextIt = std::next(it);
                if (nextIt == actingPlayers->end())
                    nextIt = actingPlayers->begin(); // Wrap around

                return *nextIt;
            }
        }

        // No big blind found (shouldn't happen), return first player
        return actingPlayers->front();
    }
    else
    {
        // Postflop: start from first player to act postflop (left of dealer)
        return getFirstPlayerToActPostFlop(hand);
    }
}

} // namespace pkt::core
