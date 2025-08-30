
#include "BettingActions.h"
#include "core/engine/HandFsm.h"
#include "core/player/PlayerFsm.h"
#include "core/player/typedefs.h"

namespace pkt::core
{
using namespace pkt::core::player;

BettingActions::BettingActions(PlayerFsmList& seats, PlayerFsmList& runningPlayers)
    : mySeatsList(seats), myRunningPlayersList(runningPlayers)
{
}
int BettingActions::getMinRaise(int smallBlind) const
{
    if (!myLastRaiserId.has_value())
    {
        // No raise yet: minimum raise is usually the big blind (preflop) or small blind
        return smallBlind;
    }

    // Compute previous raise amount
    int lastRaiserTotal = 0;
    for (auto player = myRunningPlayersList->begin(); player != myRunningPlayersList->end(); ++player)
    {
        if ((*player)->getId() == myLastRaiserId.value())
        {
            lastRaiserTotal = (*player)->getTotalBetAmount();
            break;
        }
    }

    int prevHighest = myHighestSet;
    int prevRaise = prevHighest - lastRaiserTotal;

    return prevRaise;
}

bool BettingActions::isRoundComplete(const HandFsm& hand) const
{
    if (isOnlyOnePlayerRemaining(hand))
        return true;

    return haveAllPlayersCalledOrFolded(hand);
}
int BettingActions::getHighestSet() const
{
    return myHighestSet;
}

void BettingActions::updateHighestSet(int amount)
{
    if (amount > myHighestSet)
        myHighestSet = amount;
}

bool BettingActions::haveAllPlayersCalledOrFolded(const HandFsm& hand) const
{

    const auto& players = hand.getRunningPlayersList();
    /*const int lastAggressorIndex = hand.getLastAggressorIndex(); // must be stored/set during betting
    const int currentBet = hand.getHighestSetThisRound();

    for (const auto& player : players)
    {
        if (player->hasFolded() || player->isAllIn())
            continue;

        if (player->getStack() == 0)
            continue; // Just in case not marked as all-in

        if (player->getCurrentBet() < currentBet)
            return false; // hasn't called

        // Optionally check if they’ve acted after last aggressor
        // (can be tracked using hand.hasActedSinceLastRaise(player))
    }*/

    return true;
}

bool BettingActions::isOnlyOnePlayerRemaining(const HandFsm& hand) const
{
    return hand.getRunningPlayersList()->size() == 1;
}

void BettingActions::setLastActionPlayerId(int theValue)
{
    myLastActionPlayerId = theValue;
    // myBoard->setLastActionPlayerId(theValue);
}

int BettingActions::getPreflopCallsNumber()
{
    int calls = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(GameState::Preflop);

        if (find(actions.begin(), actions.end(), ActionType::Call) != actions.end())
        {
            calls++;
        }
    }
    return calls;
}
int BettingActions::getPreflopRaisesNumber()
{

    int raises = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(GameState::Preflop);

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin)
            {
                raises++;
            }
        }
    }

    return raises;
}
int BettingActions::getFlopBetsOrRaisesNumber()
{

    int bets = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(GameState::Flop);

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin || (*itAction) == ActionType::Bet)
            {
                bets++;
            }
        }
    }

    return bets;
}
int BettingActions::getTurnBetsOrRaisesNumber()
{

    int bets = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(GameState::Turn);

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin || (*itAction) == ActionType::Bet)
            {
                bets++;
            }
        }
    }

    return bets;
}
int BettingActions::getRiverBetsOrRaisesNumber()
{

    int bets = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(GameState::River);

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin || (*itAction) == ActionType::Bet)
            {
                bets++;
            }
        }
    }

    return bets;
}
std::vector<PlayerPosition> BettingActions::getRaisersPositions()
{

    std::vector<PlayerPosition> positions;

    for (auto itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    { // note that all in players are not "running" any more

        if ((*itC)->getAction() == ActionType::Raise || (*itC)->getAction() == ActionType::Allin)
        {
            positions.push_back((*itC)->getPosition());
        }
    }
    return positions;
}

std::vector<PlayerPosition> BettingActions::getCallersPositions()
{

    std::vector<PlayerPosition> positions;

    for (auto itC = myRunningPlayersList->begin(); itC != myRunningPlayersList->end(); ++itC)
    {

        if ((*itC)->getAction() == ActionType::Call)
        {
            positions.push_back((*itC)->getPosition());
        }
    }
    return positions;
}
int BettingActions::getLastRaiserId()
{

    auto lastRaiser = mySeatsList->end();

    auto players = mySeatsList;

    for (auto it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getAction() == ActionType::Raise || (*it)->getAction() == ActionType::Allin)
        {

            if (lastRaiser != mySeatsList->end())
            {
                if ((*lastRaiser)->getPosition() < (*it)->getPosition())
                {
                    lastRaiser = it;
                }
            }
            else
            {
                lastRaiser = it;
            }
        }
    }
    if (lastRaiser != mySeatsList->end())
    {
        return (*lastRaiser)->getId();
    }

    // if no raiser was found, look for the one who have bet (if any)

    for (auto it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getAction() == ActionType::Bet)
        {
            lastRaiser = it;
        }
    }
    if (lastRaiser != mySeatsList->end())
    {
        return (*lastRaiser)->getId();
    }
    else
    {
        return -1;
    }
}
int BettingActions::getPreflopLastRaiserId()
{
    return myPreflopLastRaiserId;
}

void BettingActions::setPreflopLastRaiserId(int id)
{
    myPreflopLastRaiserId = id;
}
int BettingActions::getFlopLastRaiserId()
{
    return myFlopLastRaiserId;
}

void BettingActions::setFlopLastRaiserId(int id)
{
    myFlopLastRaiserId = id;
}
int BettingActions::getTurnLastRaiserId()
{
    return myTurnLastRaiserId;
}

void BettingActions::setTurnLastRaiserId(int id)
{
    myTurnLastRaiserId = id;
}

} // namespace pkt::core
