
#include "BettingRoundActions.h"
#include "core/engine/HandFsm.h"
#include "core/player/PlayerFsm.h"
#include "core/player/typedefs.h"

namespace pkt::core
{
using namespace pkt::core::player;

BettingRoundActions::BettingRoundActions(GameState gameState, PlayerFsmList& seats, PlayerFsmList& runningPlayers)
    : myGameState(gameState), mySeatsList(seats), myRunningPlayersList(runningPlayers)
{
}

int BettingRoundActions::getCallsNumber()
{
    int calls = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(myGameState);

        if (find(actions.begin(), actions.end(), ActionType::Call) != actions.end())
        {
            calls++;
        }
    }
    return calls;
}
int BettingRoundActions::getRaisesNumber()
{

    int raises = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(myGameState);

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
int BettingRoundActions::getBetsOrRaisesNumber()
{

    int bets = 0;

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getActions(myGameState);

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

int BettingRoundActions::getLastRaiserId()
{
    return myLastRaiserId;
}

void BettingRoundActions::setLastRaiserId(int id)
{
    myLastRaiserId = id;
}

} // namespace pkt::core
