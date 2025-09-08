
#include "BettingRoundActions.h"
#include "core/engine/HandFsm.h"
#include "core/player/PlayerFsm.h"
#include "core/player/typedefs.h"

namespace pkt::core
{
using namespace pkt::core::player;

BettingRoundActions::BettingRoundActions(GameState gameState, PlayerFsmList& seats, PlayerFsmList& actingPlayers)
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

int BettingRoundActions::getLastRaiserId()
{
    return myLastRaiserId;
}

void BettingRoundActions::setLastRaiserId(int id)
{
    myLastRaiserId = id;
}

} // namespace pkt::core
