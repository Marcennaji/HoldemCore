
#include "BettingActions.h"
#include "core/engine/HandFsm.h"
#include "core/player/PlayerFsm.h"
#include "core/player/typedefs.h"

namespace pkt::core
{
using namespace pkt::core::player;

BettingActions::BettingActions(PlayerFsmList& seats, PlayerFsmList& runningPlayers)
    : mySeatsList(seats), myRunningPlayersList(runningPlayers), myPreflop(GameState::Preflop, seats, runningPlayers),
      myFlop(GameState::Flop, seats, runningPlayers), myTurn(GameState::Turn, seats, runningPlayers),
      myRiver(GameState::River, seats, runningPlayers)
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

int BettingActions::getHighestSet() const
{
    return myHighestSet;
}

void BettingActions::updateHighestSet(int amount)
{
    if (amount > myHighestSet)
        myHighestSet = amount;
}

void BettingActions::setLastActionPlayerId(int theValue)
{
    myLastActionPlayerId = theValue;
    // myBoard->setLastActionPlayerId(theValue);
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
} // namespace pkt::core
