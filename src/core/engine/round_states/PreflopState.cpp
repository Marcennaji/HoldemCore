#include "PreflopState.h"
#include "FlopState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "core/engine/Exception.h"
#include "core/engine/model/PlayerAction.h"
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
    hand.getBettingState()->updateHighestSet(2 * mySmallBlind);
    assignButtons(hand);
    setBlinds(hand);

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(Preflop);
}

void PreflopState::exit(HandFsm& /*hand*/)
{
    // No exit action needed for Preflop
}

bool PreflopState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    auto player = getPlayerFsmById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
    {
        GlobalServices::instance().logger()->error("PreflopState: Player " + std::to_string(action.playerId) +
                                                   " not found");
        return false;
    }

    const int currentHighestBet = hand.getBettingState()->getHighestSet();
    const int playerBet = player->getTotalBetAmount();

    switch (action.type)
    {
    case ActionType::Fold:
        return true;

    case ActionType::Check:
        return playerBet == currentHighestBet && action.amount == 0;

    case ActionType::Call:
    {
        const int amountToCall = currentHighestBet - playerBet;
        return action.amount == amountToCall && player->getCash() >= amountToCall;
    }

    case ActionType::Bet:
        return currentHighestBet == 0 && action.amount > 0 && action.amount <= player->getCash();

    case ActionType::Raise:
    {
        if (action.amount <= currentHighestBet)
            return false;

        int minRaise = hand.getBettingState()->getMinRaise(mySmallBlind);
        if (action.amount < currentHighestBet + minRaise)
            return false;

        const int extraChipsRequired = action.amount - playerBet;
        if (extraChipsRequired > player->getCash())
            return false;

        return true;
    }

    case ActionType::Allin:
        return player->getCash() > 0;

    default:
        return false;
    }
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
        if ((*itC)->getTotalBetAmount() != hand.getBettingState()->getHighestSet())
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
void PreflopState::assignButtons(HandFsm& hand)
{

    size_t i;
    PlayerFsmListIterator it;
    PlayerFsmListConstIterator itC;

    // delete all buttons
    for (it = hand.getSeatsList()->begin(); it != hand.getSeatsList()->end(); ++it)
    {
        (*it)->setButton(Button::Unspecified);
    }

    // assign dealer button
    it = getPlayerFsmListIteratorById(hand.getSeatsList(), myDealerPlayerId);
    if (it == hand.getSeatsList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }
    (*it)->setButton(Dealer);

    // assign Small Blind next to dealer. ATTENTION: in heads up it is big blind
    // assign big blind next to small blind. ATTENTION: in heads up it is small blind
    bool nextActivePlayerFound = false;
    auto dealerPositionIt = getPlayerFsmListIteratorById(hand.getSeatsList(), myDealerPlayerId);
    if (dealerPositionIt == hand.getSeatsList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }

    for (i = 0; i < hand.getSeatsList()->size(); i++)
    {

        ++dealerPositionIt;
        if (dealerPositionIt == hand.getSeatsList()->end())
        {
            dealerPositionIt = hand.getSeatsList()->begin();
        }
        it = getPlayerFsmListIteratorById(hand.getSeatsList(), (*dealerPositionIt)->getId());
        if (it != hand.getSeatsList()->end())
        {
            nextActivePlayerFound = true;
            if (hand.getSeatsList()->size() > 2)
            {
                // small blind normal
                (*it)->setButton(SmallBlind);
                mySmallBlindPlayerId = (*it)->getId();
            }
            else
            {
                // big blind in heads up
                (*it)->setButton(BigBlind);
                myBigBlindPlayerId = (*it)->getId();
                // lastPlayerAction for showing cards
            }

            // first player after dealer have to show his cards first (in showdown)
            // myLastActionPlayerId = (*it)->getId();
            // myBoard->setLastActionPlayerId(myLastActionPlayerId);

            ++it;
            if (it == hand.getSeatsList()->end())
            {
                it = hand.getSeatsList()->begin();
            }

            if (hand.getSeatsList()->size() > 2)
            {
                // big blind normal
                (*it)->setButton(BigBlind);
                myBigBlindPlayerId = (*it)->getId();
            }
            else
            {
                // small blind in heads up
                (*it)->setButton(SmallBlind);
                mySmallBlindPlayerId = (*it)->getId();
            }

            break;
        }
    }
    if (!nextActivePlayerFound)
    {
        throw Exception(__FILE__, __LINE__, EngineError::NextActivePlayerNotFound);
    }
}

void PreflopState::setBlinds(HandFsm& hand)
{
    PlayerFsmListConstIterator itC;

    for (itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {

        // small blind
        if ((*itC)->getButton() == SmallBlind)
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
        if ((*itC)->getButton() == BigBlind)
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
