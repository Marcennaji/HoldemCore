
#include "BettingRoundActions.h"
#include "../hand/Hand.h"
#include "core/player/Player.h"
#include "core/player/typedefs.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{
using namespace pkt::core::player;

BettingRoundActions::BettingRoundActions(GameState gameState, PlayerList& seats, PlayerList& actingPlayers)
    : myGameState(gameState), mySeatsList(seats), myActingPlayersList(actingPlayers)
{
}

int BettingRoundActions::getCallsNumber()
{
    int calls = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        auto& actions = (*it)->getCurrentHandActions().getActions(myGameState);

        for (auto itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if (itAction->type == ActionType::Call)
            {
                calls++;
            }
        }
    }
    return calls;
}
int BettingRoundActions::getRaisesNumber()
{

    int raises = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        auto& actions = (*it)->getCurrentHandActions().getActions(myGameState);

        for (auto itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if (itAction->type == ActionType::Raise || itAction->type == ActionType::Allin)
            {
                raises++;
            }
        }
    }

    return raises;
}
int BettingRoundActions::getBetsOrRaisesNumber()
{

    int bets = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        auto& actions = (*it)->getCurrentHandActions().getActions(myGameState);

        for (auto itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if (itAction->type == ActionType::Raise || itAction->type == ActionType::Allin ||
                itAction->type == ActionType::Bet)
            {
                bets++;
            }
        }
    }

    return bets;
}

std::shared_ptr<pkt::core::player::Player> BettingRoundActions::getLastRaiser()
{
    // If we have a stored last raiser, return it
    if (myLastRaiser)
    {
        return myLastRaiser;
    }

    // If no stored last raiser but there are raises, find the last raiser dynamically
    // This ensures consistency when raises exist but lastRaiser wasn't properly set
    std::shared_ptr<pkt::core::player::Player> lastRaiser = nullptr;
    
    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        auto& actions = (*it)->getCurrentHandActions().getActions(myGameState);

        for (auto itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if (itAction->type == ActionType::Raise || itAction->type == ActionType::Allin)
            {
                lastRaiser = *it;
            }
        }
    }

    // Cache the found last raiser to avoid recalculation
    if (lastRaiser)
    {
        myLastRaiser = lastRaiser;
    }

    return lastRaiser;
}

void BettingRoundActions::setLastRaiser(std::shared_ptr<pkt::core::player::Player> player)
{
    myLastRaiser = player;
}

} // namespace pkt::core
