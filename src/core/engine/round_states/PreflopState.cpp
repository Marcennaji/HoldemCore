#include "PreflopState.h"

#include "FlopState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "core/engine/Exception.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/BotPlayer.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace pkt::core::player;

PreflopState::PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId)
    : myEvents(events), mySmallBlind(smallBlind), myDealerPlayerId(dealerPlayerId)
{
}

void PreflopState::enter(HandFsm& hand)
{
    myHighestSet = 2 * mySmallBlind;

    assignButtons(hand);

    setBlinds(hand);

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(GameStatePreflop);
}

void PreflopState::exit(HandFsm& /*hand*/)
{
    // No exit action needed for Preflop
}

bool PreflopState::canProcessAction(const HandFsm& hand, const PlayerAction action) const
{
    auto player = getPlayerFsmById(hand.getRunningPlayersList(), action.playerId);
    if (!player)
        return false;

    const int cash = player->getLegacyPlayer()->getCash();

    const int amountToCall = getHighestSet() - player->getLegacyPlayer()->getSet();

    switch (action.type)
    {
    case ActionType::Fold:
        return true;

    case ActionType::Check:
        return action.amount == 0;

    case ActionType::Call:
        return action.amount > 0 && cash >= action.amount;

    case ActionType::Bet:
        return action.amount > 0 && action.amount <= cash;

    case ActionType::Raise:
        // TODO: enforce minimum raise rules here
        return action.amount <= cash;

    case ActionType::Allin:
        return cash > 0;

    default:
        return false;
    }

    return false;
}

void PreflopState::promptPlayerAction(HandFsm& hand, Player& player)
{
    if (!player.isBot())
        return;

    auto& bot = static_cast<BotPlayer&>(player);
    const PlayerAction action = bot.decidePreflopActionFsm();
    hand.processPlayerAction(action);
}

std::unique_ptr<IHandState> PreflopState::processAction(HandFsm& hand, PlayerAction action)
{
    if (isRoundComplete(hand))
    {
        exit(hand);
        return std::make_unique<FlopState>(myEvents);
    }

    return nullptr;
}

bool PreflopState::isRoundComplete(const HandFsm& hand) const
{
    int highestSet = -1;

    if (hand.getRunningPlayersList()->size() <= 1)
        return true;

    for (auto itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {
        if (highestSet == -1)
            highestSet = (*itC)->getLegacyPlayer()->getSet();
        else
        {
            if (highestSet != (*itC)->getLegacyPlayer()->getSet())
            {
                return false;
            }
        }
    }

    return true;
}

void PreflopState::logStateInfo(const HandFsm& /*hand*/) const
{
    // Optional: Add logging when debugging
}
void PreflopState::assignButtons(HandFsm& hand)
{

    size_t i;
    PlayerFsmListIterator it;
    PlayerFsmListConstIterator itC;

    // delete all buttons
    for (it = hand.getSeatsList()->begin(); it != hand.getSeatsList()->end(); ++it)
    {
        (*it)->setButton(ButtonNone);
    }

    // assign dealer button
    it = getPlayerFsmListIteratorById(hand.getSeatsList(), myDealerPlayerId);
    if (it == hand.getSeatsList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }
    (*it)->setButton(ButtonDealer);

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
                (*it)->setButton(ButtonSmallBlind);
                mySmallBlindPlayerId = (*it)->getId();
            }
            else
            {
                // big blind in heads up
                (*it)->setButton(ButtonBigBlind);
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
                (*it)->setButton(ButtonBigBlind);
                myBigBlindPlayerId = (*it)->getId();
            }
            else
            {
                // small blind in heads up
                (*it)->setButton(ButtonSmallBlind);
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
        if ((*itC)->getButton() == ButtonSmallBlind)
        {

            // All in ?
            if ((*itC)->getLegacyPlayer()->getCash() <= mySmallBlind)
            {

                (*itC)->getLegacyPlayer()->setSet((*itC)->getLegacyPlayer()->getCash());
                // 1 to do not log this
                (*itC)->getLegacyPlayer()->setAction(ActionType::Allin, 1);
            }
            else
            {
                (*itC)->getLegacyPlayer()->setSet(mySmallBlind);
            }
        }
    }

    for (itC = hand.getRunningPlayersList()->begin(); itC != hand.getRunningPlayersList()->end(); ++itC)
    {

        // big blind
        if ((*itC)->getButton() == ButtonBigBlind)
        {

            // all in ?
            if ((*itC)->getLegacyPlayer()->getCash() <= 2 * mySmallBlind)
            {

                (*itC)->getLegacyPlayer()->setSet((*itC)->getLegacyPlayer()->getCash());
                // 1 to do not log this
                (*itC)->getLegacyPlayer()->setAction(ActionType::Allin, 1);
            }
            else
            {
                (*itC)->getLegacyPlayer()->setSet(2 * mySmallBlind);
            }
        }
    }
}
} // namespace pkt::core
