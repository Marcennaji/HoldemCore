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
}

void PreflopState::enter(HandFsm& hand)
{
    hand.getBettingActions()->updateHighestSet(2 * mySmallBlind);
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
    auto player = getPlayerFsmById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
    {
        GlobalServices::instance().logger().error("PreflopState: Player " + std::to_string(action.playerId) +
                                                  " not found");
        return false;
    }
    return validatePlayerAction(*player, action, *hand.getBettingActions(), mySmallBlind);
}

void PreflopState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(Preflop, hand);
    const PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> PreflopState::computeNextState(HandFsm& hand, PlayerAction action)
{
    if (hand.getRunningPlayersList()->size() == 1)
    {
        exit(hand);
        return std::make_unique<PostRiverState>(myEvents);
    }
    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<FlopState>(myEvents);
    }

    return nullptr;
}

bool PreflopState::isRoundComplete(const HandFsm& hand) const
{
    if (hand.getRunningPlayersList()->size() <= 1)
        return true;

    for (auto itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {
        if ((*itC)->getTotalBetAmount() != hand.getBettingActions()->getHighestSet())
        {
            return false;
        }
    }

    return true;
}

void PreflopState::logStateInfo(const HandFsm& /*hand*/) const
{
    // todo
}

void PreflopState::setBlinds(HandFsm& hand)
{
    PlayerFsmListConstIterator itC;

    for (itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {

        // small blind
        if ((*itC)->getPosition() == PlayerPosition::SmallBlind ||
            (*itC)->getPosition() == PlayerPosition::ButtonSmallBlind)
        {

            // All in ?
            if ((*itC)->getCash() <= mySmallBlind)
            {

                (*itC)->addBetAmount((*itC)->getCash());
                // 1 to do not log this
                (*itC)->setAction(ActionType::Allin, 1);
            }
            else
            {
                (*itC)->addBetAmount(mySmallBlind);
            }
        }
    }

    for (itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {

        // big blind
        if ((*itC)->getPosition() == PlayerPosition::BigBlind)
        {

            // all in ?
            if ((*itC)->getCash() <= 2 * mySmallBlind)
            {

                (*itC)->addBetAmount((*itC)->getCash());
                // 1 to do not log this
                (*itC)->setAction(ActionType::Allin, 1);
            }
            else
            {
                (*itC)->addBetAmount(2 * mySmallBlind);
            }
        }
    }
}
} // namespace pkt::core
