#include "PreflopState.h"
#include "FlopState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "core/engine/Exception.h"
#include "core/engine/Helpers.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Player.h"
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

void PreflopState::enter(Hand& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    hand.getBettingActions()->updateRoundHighestSet(2 * mySmallBlind);
    setBlinds(hand);

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(Preflop);

    logStateInfo(hand);
}

void PreflopState::exit(Hand& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->updateCurrentHandContext(GameState::Preflop, hand);
        player->getStatisticsUpdater()->updatePreflopStatistics(player->getCurrentHandContext());
    }
}

bool PreflopState::isActionAllowed(const Hand& hand, const PlayerAction action) const
{
    return (
        validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), mySmallBlind, Preflop));
}

void PreflopState::promptPlayerAction(Hand& hand, Player& player)
{
    player.updateCurrentHandContext(Preflop, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> PreflopState::computeNextState(Hand& hand)
{
    return computeBettingRoundNextState(hand, myEvents, Preflop);
}

void PreflopState::logStateInfo(Hand& hand)
{
    IDebuggableState::logStateInfo(hand);
    GlobalServices::instance().logger().info("Blinds: SB=" + std::to_string(mySmallBlind) +
                                             ", BB=" + std::to_string(2 * mySmallBlind));
    logHoleCards(hand);
}

void PreflopState::logHoleCards(Hand& hand)
{
    for (const auto& player : *hand.getSeatsList())
    {
        const HoleCards& holeCards = player->getHoleCards();
        if (holeCards.isValid())
        {
            GlobalServices::instance().logger().info("Player " + std::to_string(player->getId()) + " (" +
                                                     player->getName() + "): " + holeCards.toString());
        }
        else
        {
            GlobalServices::instance().logger().info("Player " + std::to_string(player->getId()) + " (" +
                                                     player->getName() + "): No hole cards");
        }
    }
}

std::shared_ptr<player::Player> PreflopState::getNextPlayerToAct(const Hand& hand) const
{
    return getNextPlayerToActInRound(hand, GameState::Preflop);
}

std::shared_ptr<player::Player> PreflopState::getFirstPlayerToActInRound(const Hand& hand) const
{
    // In preflop, the first player to act is left of the big blind
    // This handles both heads-up (SB acts first) and multi-player (UTG acts first)
    return getNextPlayerToAct(hand);
}

bool PreflopState::isRoundComplete(const Hand& hand) const
{

    return pkt::core::isRoundComplete(const_cast<Hand&>(hand));
}

void PreflopState::setBlinds(Hand& hand)
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

            if (myEvents.onPlayerActed)
            {
                myEvents.onPlayerActed(blindAction);
            }
        }
    }
}
} // namespace pkt::core
