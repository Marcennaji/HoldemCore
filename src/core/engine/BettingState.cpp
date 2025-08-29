
#include "BettingState.h"
#include "core/engine/HandFsm.h"
#include "core/player/PlayerFsm.h"
#include "core/player/typedefs.h"

namespace pkt::core
{
using namespace pkt::core::player;

BettingState::BettingState(PlayerFsmList& seats, PlayerFsmList& runningPlayers)
    : mySeatsList(seats), myRunningPlayersList(runningPlayers)
{
}

bool BettingState::isRoundComplete(const HandFsm& hand) const
{
    if (isOnlyOnePlayerRemaining(hand))
        return true;

    return haveAllPlayersCalledOrFolded(hand);
}
int BettingState::getHighestSet() const
{
    return myHighestSet;
}

void BettingState::updateHighestSet(int amount)
{
    if (amount > myHighestSet)
        myHighestSet = amount;
}

bool BettingState::haveAllPlayersCalledOrFolded(const HandFsm& hand) const
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

bool BettingState::isOnlyOnePlayerRemaining(const HandFsm& hand) const
{
    return hand.getRunningPlayersList()->size() == 1;
}

void BettingState::setLastActionPlayerId(int theValue)
{
    myLastActionPlayerId = theValue;
    // myBoard->setLastActionPlayerId(theValue);
}

int BettingState::getPreflopCallsNumber()
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
int BettingState::getPreflopRaisesNumber()
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
int BettingState::getFlopBetsOrRaisesNumber()
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
int BettingState::getTurnBetsOrRaisesNumber()
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
int BettingState::getRiverBetsOrRaisesNumber()
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
std::vector<PlayerPosition> BettingState::getRaisersPositions()
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

std::vector<PlayerPosition> BettingState::getCallersPositions()
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
int BettingState::getLastRaiserId()
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
int BettingState::getPreflopLastRaiserId()
{
    return myPreflopLastRaiserId;
}

void BettingState::setPreflopLastRaiserId(int id)
{
    myPreflopLastRaiserId = id;
}
int BettingState::getFlopLastRaiserId()
{
    return myFlopLastRaiserId;
}

void BettingState::setFlopLastRaiserId(int id)
{
    myFlopLastRaiserId = id;
}
int BettingState::getTurnLastRaiserId()
{
    return myTurnLastRaiserId;
}

void BettingState::setTurnLastRaiserId(int id)
{
    myTurnLastRaiserId = id;
}

} // namespace pkt::core
