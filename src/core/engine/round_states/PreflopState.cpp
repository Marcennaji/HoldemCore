#include "PreflopState.h"
#include "FlopState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "core/engine/Exception.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"
#include "core/player/PlayerFsm.h"
#include "core/player/deprecated/Player.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace pkt::core::player;

PreflopState::PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId)
    : myEvents(events), mySmallBlind(smallBlind), myDealerPlayerId(dealerPlayerId)
{
    if (smallBlind <= 0)
    {
        throw std::invalid_argument("PreflopState: smallBlind must be > 0");
    }

    if (dealerPlayerId == static_cast<unsigned>(-1))
    {
        throw std::invalid_argument("PreflopState: dealerPlayerId is invalid");
    }
}

void PreflopState::enter(HandFsm& hand)
{
    GlobalServices::instance().logger().info("Preflop");

    for (auto& player : *hand.getSeatsList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    hand.getBettingActions()->updateRoundHighestSet(2 * mySmallBlind);
    setBlinds(hand);

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(Preflop);
}

void PreflopState::exit(HandFsm& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->updateCurrentHandContext(GameState::Preflop, hand);
        player->getStatisticsUpdater()->updatePreflopStatistics(player->getCurrentHandContext());
    }
}

bool PreflopState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (
        validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), mySmallBlind, Preflop));
}

void PreflopState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Preflop, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> PreflopState::computeNextState(HandFsm& hand)
{
    return computeBettingRoundNextState(hand, myEvents, Preflop);
}

void PreflopState::logStateInfo(const HandFsm& /*hand*/) const
{
    // todo
}

std::shared_ptr<player::PlayerFsm> PreflopState::getNextPlayerToAct(const HandFsm& hand) const
{
    // In preflop, the first to act is left of the big blind (UTG in multi-player, SB in heads-up)
    auto actingPlayers = hand.getActingPlayersList();

    if (actingPlayers->empty())
        return nullptr;

    // Find the big blind player in the acting players list
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

std::shared_ptr<player::PlayerFsm> PreflopState::getFirstPlayerToActInRound(const HandFsm& hand) const
{
    // In preflop, the first player to act is left of the big blind
    // This handles both heads-up (SB acts first) and multi-player (UTG acts first)
    return getNextPlayerToAct(hand);
}

bool PreflopState::isRoundComplete(const HandFsm& hand) const
{

    return pkt::core::player::isRoundComplete(const_cast<HandFsm&>(hand));
}

void PreflopState::setBlinds(HandFsm& hand)
{
    for (const auto& player : *hand.getActingPlayersList())
    {
        int blindAmount = 0;
        ActionType actionType = ActionType::None;

        if (player->getPosition() == PlayerPosition::SmallBlind ||
            player->getPosition() == PlayerPosition::ButtonSmallBlind)
        {
            blindAmount = mySmallBlind;
            actionType = ActionType::PostSmallBlind;
        }
        else if (player->getPosition() == PlayerPosition::BigBlind)
        {
            blindAmount = 2 * mySmallBlind;
            actionType = ActionType::PostBigBlind;
        }

        if (blindAmount > 0)
        {
            PlayerAction blindAction;
            if (player->getCash() < blindAmount)
            {
                // Player doesn't have enough cash to post the blind, goes all-in
                player->addBetAmount(player->getCash());
                blindAction = {player->getId(), ActionType::Allin, player->getCash()};
                player->setAction(*this, blindAction);
            }
            else
            {
                // Player can afford the blind
                player->addBetAmount(blindAmount);
                blindAction = {player->getId(), actionType, blindAmount};
                player->setAction(*this, blindAction);
            }

            // Record blind post in hand-level chronological history
            hand.getBettingActions()->recordPlayerAction(getGameState(), blindAction);
        }
    }
}
} // namespace pkt::core
