
#include "Helpers.h"
#include "actions/ActionValidator.h"
#include "actions/BettingActions.h"
#include "core/engine/Exception.h"
#include "core/engine/cards/CardUtilities.h"
#include "core/engine/hand/Hand.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"
// Include FSM states for the helper function
#include "core/engine/state/FlopState.h"
#include "core/engine/state/PostRiverState.h"
#include "core/engine/state/RiverState.h"
#include "core/engine/state/TurnState.h"

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

    // Debug logging to help diagnose invalid action reports
    {
        static std::shared_ptr<pkt::core::ServiceContainer> s_defaultServices =
            std::make_shared<pkt::core::AppServiceContainer>();
        std::string actionsStr;
        for (size_t i = 0; i < validActions.size(); ++i)
        {
            actionsStr += actionTypeToString(validActions[i]);
            if (i + 1 < validActions.size()) actionsStr += ",";
        }
        s_defaultServices->logger().debug(
            "valid actions for player " + player->getName() + " on " + gameStateToString(gameState) +
            ": highest=" + std::to_string(currentHighestBet) + ", bet=" + std::to_string(playerBet) +
            ", cash=" + std::to_string(playerCash) + " => [" + actionsStr + "]");
    }

    return validActions;
}

std::shared_ptr<player::Player> getFirstPlayerToActPostFlop(const Hand& hand)
{
    auto actingPlayers = hand.getActingPlayersList();
    auto seats = hand.getSeatsList();

    if (actingPlayers->empty() || seats->empty())
        return nullptr;

    // Determine the first active player to the left of the dealer, wrapping around.
    unsigned dealerId = hand.getDealerPlayerId();
    auto dealerIt = getPlayerListIteratorById(seats, dealerId);

    // If we can't find the dealer in seats (shouldn't happen), fallback to first active player
    if (dealerIt == seats->end())
        return actingPlayers->front();

    auto it = dealerIt;
    for (size_t i = 0; i < seats->size(); ++i)
    {
        ++it;
        if (it == seats->end())
            it = seats->begin();

        // Is this seated player still active (i.e., present in actingPlayers)?
        auto activeIt = getPlayerListIteratorById(actingPlayers, (*it)->getId());
        if (activeIt != actingPlayers->end())
            return *activeIt;
    }

    // Fallback: return first active player
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

    // If all remaining players are all-in (no one can act further), we should still
    // progress through the normal betting round states to show card progression,
    // but deal the appropriate cards for the next state and skip betting actions.
    if (hand.getActingPlayersList()->empty() && hand.getPlayersInHandList()->size() >= 1)
    {
        // Inform board that showdown is due to all-in; affects reveal ordering later.
        hand.getBoard().setAllInCondition(true);

        // Determine next state and deal appropriate cards based on current state
        switch (currentState)
        {
        case pkt::core::GameState::Preflop:
        {
            // Deal flop cards
            BoardCards board = hand.getBoard().getBoardCards();
            if (board.getNumCards() == 0)
            {
                auto flopCards = hand.dealCardsFromDeck(3);
                BoardCards flopBoard(flopCards[0], flopCards[1], flopCards[2]);
                hand.getBoard().setBoardCards(flopBoard);
                if (events.onBoardCardsDealt) events.onBoardCardsDealt(flopBoard);
            }
            return std::make_unique<pkt::core::FlopState>(events);
        }
        case pkt::core::GameState::Flop:
        {
            // Deal turn card
            BoardCards board = hand.getBoard().getBoardCards();
            if (board.getNumCards() == 3)
            {
                auto turnCards = hand.dealCardsFromDeck(1);
                BoardCards turnBoard = board;
                turnBoard.dealTurn(turnCards[0]);
                hand.getBoard().setBoardCards(turnBoard);
                if (events.onBoardCardsDealt) events.onBoardCardsDealt(turnBoard);
            }
            return std::make_unique<pkt::core::TurnState>(events);
        }
        case pkt::core::GameState::Turn:
        {
            // Deal river card
            BoardCards board = hand.getBoard().getBoardCards();
            if (board.getNumCards() == 4)
            {
                auto riverCards = hand.dealCardsFromDeck(1);
                BoardCards riverBoard = board;
                riverBoard.dealRiver(riverCards[0]);
                hand.getBoard().setBoardCards(riverBoard);
                if (events.onBoardCardsDealt) events.onBoardCardsDealt(riverBoard);
            }
            return std::make_unique<pkt::core::RiverState>(events);
        }
        case pkt::core::GameState::River:
        {
            // All cards dealt, proceed to showdown
            return std::make_unique<pkt::core::PostRiverState>(events);
        }
        default:
            return std::make_unique<pkt::core::PostRiverState>(events);
        }
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
    // Prefer using the same services as the hand/state manager when possible to keep logging/config consistent.
    // Fallback to a shared AppServiceContainer instance only if not available.
    static std::shared_ptr<pkt::core::ServiceContainer> s_defaultServices =
        std::make_shared<pkt::core::AppServiceContainer>();
    return isRoundComplete(hand, s_defaultServices);
}

bool isRoundComplete(const Hand& hand, std::shared_ptr<pkt::core::ServiceContainer> services)
{
    assert(hand.getGameState() != GameState::None);

    for (auto player = hand.getActingPlayersList()->begin(); player != hand.getActingPlayersList()->end(); ++player)
    {
        services->logger().verbose("checking if round " + gameStateToString(hand.getGameState()) +
                                   " is complete : Checking player: " + (*player)->getName());

        if ((*player)->getLastAction().type == ActionType::None ||
            (*player)->getLastAction().type == ActionType::PostBigBlind ||
            (*player)->getLastAction().type == ActionType::PostSmallBlind)
        {
            services->logger().verbose("  ROUND NOT COMPLETE, as player " + (*player)->getName() + " did not act.");
            return false;
        }

        services->logger().verbose(
            "  player round bet amount: " +
            std::to_string((*player)->getCurrentHandActions().getRoundTotalBetAmount(hand.getGameState())) +
            ", hand total bet amount : " + std::to_string((*player)->getCurrentHandActions().getHandTotalBetAmount()) +
            " vs current round highest bet: " + std::to_string(hand.getBettingActions()->getRoundHighestSet()));

        if ((*player)->getCurrentHandActions().getRoundTotalBetAmount(hand.getGameState()) <
            hand.getBettingActions()->getRoundHighestSet())
        {
            services->logger().verbose("  ROUND NOT COMPLETE, as player " + (*player)->getName() +
                                       " has not matched the highest bet yet.");
            return false;
        }
    }
    services->logger().verbose("  ROUND " + gameStateToString(hand.getGameState()) + " COMPLETE");
    return true;
}

// Legacy function for backwards compatibility - delegates to ActionValidator
bool validatePlayerAction(const PlayerList& actingPlayersList, const PlayerAction& action,
                          const BettingActions& bettingActions, int smallBlind, const GameState gameState)
{
    return ActionValidator::validate(actingPlayersList, action, bettingActions, smallBlind, gameState);
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
