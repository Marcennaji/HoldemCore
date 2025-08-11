
#include "PlayerStatisticsUpdater.h"
#include "core/player/PlayerFsm.h"
#include "core/player/strategy/CurrentHandContext.h"
#include "core/services/GlobalServices.h"

namespace pkt::core::player
{

const PlayerStatistics& PlayerStatisticsUpdater::getStatistics(const int nbPlayers) const
{
    return myStatistics[nbPlayers];
}
void PlayerStatisticsUpdater::resetPlayerStatistics()
{

    for (int i = 0; i <= MAX_NUMBER_OF_PLAYERS; i++)
    {
        myStatistics[i].reset();
    }
}

void PlayerStatisticsUpdater::updatePreflopStatistics(const CurrentHandContext& ctx, const ActionType& action)
{
    const int nbPlayers = ctx.commonContext.nbPlayers;

    if (ctx.perPlayerContext.myCurrentHandActions.getPreflopActions().size() == 1)
    {
        myStatistics[nbPlayers].totalHands++;
        myStatistics[nbPlayers].preflopStatistics.hands++;
    }

    switch (action)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].preflopStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].preflopStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].preflopStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].preflopStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].preflopStatistics.calls++;
        break;
    default:
        break;
    }

    myStatistics[nbPlayers].preflopStatistics.addLastAction(action); // keep track of the last 10 actions

    if (action == ActionType::Call && ctx.commonContext.raisersPositions.size() == 0)
    { //
        myStatistics[nbPlayers].preflopStatistics.limps++;
    }

    int playerRaises = 0;
    for (auto i = ctx.perPlayerContext.myCurrentHandActions.getPreflopActions().begin();
         i != ctx.perPlayerContext.myCurrentHandActions.getPreflopActions().end(); i++)
    {
        if (*i == ActionType::Raise || *i == ActionType::Allin)
        {
            playerRaises++;
        }
    }

    if (action == ActionType::Raise || action == ActionType::Allin)
    {

        if (playerRaises == 1 && ctx.commonContext.raisersPositions.size() == 2)
        {
            myStatistics[nbPlayers].preflopStatistics.threeBets++;
        }

        if (playerRaises == 2 || (playerRaises == 1 && ctx.commonContext.raisersPositions.size() == 3))
        {
            myStatistics[nbPlayers].preflopStatistics.fourBets++;
        }
    }
    else
    {

        if (playerRaises == 1)
        {

            myStatistics[nbPlayers].preflopStatistics.callthreeBetsOpportunities++;

            if (action == ActionType::Call)
            {
                myStatistics[nbPlayers].preflopStatistics.callthreeBets++;
            }
        }
    }
}
void PlayerStatisticsUpdater::updateFlopStatistics(const CurrentHandContext& ctx, const ActionType& action)
{
    const int nbPlayers = ctx.commonContext.nbPlayers;

    if (ctx.perPlayerContext.myCurrentHandActions.getFlopActions().size() == 1)
    {
        myStatistics[nbPlayers].flopStatistics.hands++;
    }

    switch (action)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].flopStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].flopStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].flopStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].flopStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].flopStatistics.calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].flopStatistics.bets++;
        break;
    default:
        break;
    }
    if (action == ActionType::Raise && ctx.commonContext.raisersPositions.size() > 1)
    {
        myStatistics[nbPlayers].flopStatistics.threeBets++;
    }

    // continuation bets
    if (ctx.commonContext.preflopLastRaiserFsm &&
        ctx.commonContext.preflopLastRaiserFsm->getId() == ctx.perPlayerContext.myID)
    {
        myStatistics[nbPlayers].flopStatistics.continuationBetsOpportunities++;
        if (action == ActionType::Bet)
        {
            myStatistics[nbPlayers].flopStatistics.continuationBets++;
        }
    }
}
void PlayerStatisticsUpdater::updateTurnStatistics(const CurrentHandContext& ctx, const ActionType& action)
{
    const int nbPlayers = ctx.commonContext.nbPlayers;

    if (ctx.perPlayerContext.myCurrentHandActions.getTurnActions().size() == 1)
    {
        myStatistics[nbPlayers].turnStatistics.hands++;
    }

    switch (action)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].turnStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].turnStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].turnStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].turnStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].turnStatistics.calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].turnStatistics.bets++;
        break;
    default:
        break;
    }
    if (action == ActionType::Raise && ctx.commonContext.raisersPositions.size() > 1)
    {
        myStatistics[nbPlayers].turnStatistics.threeBets++;
    }
}
void PlayerStatisticsUpdater::updateRiverStatistics(const CurrentHandContext& ctx, const ActionType& action)
{
    const int nbPlayers = ctx.commonContext.nbPlayers;

    if (ctx.perPlayerContext.myCurrentHandActions.getRiverActions().size() == 1)
    {
        myStatistics[nbPlayers].riverStatistics.hands++;
    }

    switch (action)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].riverStatistics.raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].riverStatistics.raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].riverStatistics.folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].riverStatistics.checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].riverStatistics.calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].riverStatistics.bets++;
        break;
    default:
        break;
    }
    if (action == ActionType::Raise && ctx.commonContext.raisersPositions.size() > 1)
    {
        myStatistics[nbPlayers].riverStatistics.threeBets++;
    }
}

void PlayerStatisticsUpdater::loadStatistics(const std::string& playerName)
{

    resetPlayerStatistics(); // reset stats to 0

    myStatistics = GlobalServices::instance().playersStatisticsStore()->getPlayerStatistics(playerName);
    if (myStatistics.empty())
    {
        myStatistics.fill(PlayerStatistics());
    }
}

} // namespace pkt::core::player
