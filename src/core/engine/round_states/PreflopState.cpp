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
    GlobalServices::instance().logger().info("PreflopState: Entering preflop");

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
    GlobalServices::instance().logger().info("PreflopState: Exiting preflop");
}

bool PreflopState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (
        validatePlayerAction(hand.getRunningPlayersList(), action, *hand.getBettingActions(), mySmallBlind, Preflop));
}

void PreflopState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Preflop, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> PreflopState::computeNextState(HandFsm& hand, PlayerAction action)
{
    if (hand.getRunningPlayersList()->size() < 2)
    {
        return std::make_unique<PostRiverState>(myEvents);
    }
    if (isRoundComplete(hand))
    {
        return std::make_unique<FlopState>(myEvents);
    }

    return nullptr;
}

void PreflopState::logStateInfo(const HandFsm& /*hand*/) const
{
    // todo
}

void PreflopState::setBlinds(HandFsm& hand)
{
    for (const auto& player : *hand.getRunningPlayersList())
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
            if (player->getCash() < blindAmount)
            {
                // Player doesn't have enough cash to post the blind, goes all-in
                player->addBetAmount(player->getCash());
                player->setAction(*this, {player->getId(), ActionType::Allin, player->getCash()});
            }
            else
            {
                // Player can afford the blind
                player->addBetAmount(blindAmount);
                player->setAction(*this, {player->getId(), actionType, blindAmount});
            }
        }
    }
}
} // namespace pkt::core
