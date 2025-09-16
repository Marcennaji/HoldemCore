
#include "BettingActions.h"
#include "core/engine/Hand.h"
#include "core/player/Player.h"
#include "core/player/typedefs.h"

#include <algorithm>

namespace pkt::core
{
using namespace pkt::core::player;

BettingActions::BettingActions(PlayerList& seats, PlayerList& actingPlayers)
    : mySeatsList(seats), myActingPlayersList(actingPlayers), myPreflop(GameState::Preflop, seats, actingPlayers),
      myFlop(GameState::Flop, seats, actingPlayers), myTurn(GameState::Turn, seats, actingPlayers),
      myRiver(GameState::River, seats, actingPlayers)
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
    for (auto player = myActingPlayersList->begin(); player != myActingPlayersList->end(); ++player)
    {
        if ((*player)->getId() == myLastRaiserId.value())
        {
            PlayerAction lastAction = (*player)->getCurrentHandActions().getLastAction();
            assert(lastAction.type == ActionType::Raise || lastAction.type == ActionType::Allin);
            lastRaiserTotal = lastAction.amount;
            break;
        }
    }

    int prevHighest = myRoundHighestSet;
    int prevRaise = prevHighest - lastRaiserTotal;

    return prevRaise;
}

int BettingActions::getRoundHighestSet() const
{
    return myRoundHighestSet;
}

void BettingActions::updateRoundHighestSet(int amount)
{
    if (amount > myRoundHighestSet)
        myRoundHighestSet = amount;
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

        if ((*itC)->getLastAction().type == ActionType::Raise || (*itC)->getLastAction().type == ActionType::Allin)
        {
            positions.push_back((*itC)->getPosition());
        }
    }
    return positions;
}

std::vector<PlayerPosition> BettingActions::getCallersPositions()
{

    std::vector<PlayerPosition> positions;

    for (auto itC = myActingPlayersList->begin(); itC != myActingPlayersList->end(); ++itC)
    {

        if ((*itC)->getLastAction().type == ActionType::Call)
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

        if ((*it)->getLastAction().type == ActionType::Raise || (*it)->getLastAction().type == ActionType::Allin)
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

        if ((*it)->getLastAction().type == ActionType::Bet)
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

void BettingActions::recordPlayerAction(GameState round, const pkt::core::PlayerAction& action)
{
    // Find or create entry for this round
    auto it = std::find_if(myHandActionHistory.begin(), myHandActionHistory.end(),
                           [round](const pkt::core::BettingRoundHistory& h) { return h.round == round; });

    if (it == myHandActionHistory.end())
    {
        myHandActionHistory.push_back({round, {{action.playerId, action.type}}});
    }
    else
    {
        it->actions.emplace_back(action.playerId, action.type);
    }
}

} // namespace pkt::core
