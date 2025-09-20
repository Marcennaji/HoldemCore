
#include "PlayerStatisticsUpdater.h"
#include <core/services/ServiceContainer.h>
#include "core/player/Player.h"
#include "core/player/strategy/CurrentHandContext.h"

namespace pkt::core::player
{
PlayerStatisticsUpdater::PlayerStatisticsUpdater(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
    : myServices(serviceContainer)
{
}

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
void PlayerStatisticsUpdater::updateStatistics(GameState state, const CurrentHandContext& ctx)
{
    switch (state)
    {
    case GameState::Preflop:
        updatePreflopStatistics(ctx);
        break;
    case GameState::Flop:
        updateFlopStatistics(ctx);
        break;
    case GameState::Turn:
        updateTurnStatistics(ctx);
        break;
    case GameState::River:
        updateRiverStatistics(ctx);
        break;
    default:
        break;
    }
}

void PlayerStatisticsUpdater::updatePreflopStatistics(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    auto actions = ctx.personalContext.actions.currentHandActions.getActions(GameState::Preflop);
    if (actions.empty())
        return;

    myStatistics[nbPlayers].totalHands++;
    myStatistics[nbPlayers].preflopStatistics.hands++;

    for (const auto& action : actions)
    {
        switch (action.type)
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

        if (action.type == ActionType::Call && ctx.commonContext.playersContext.raisersPositions.size() == 0)
        { //
            myStatistics[nbPlayers].preflopStatistics.limps++;
        }

        int playerRaises = 0;
        for (auto i = ctx.personalContext.actions.currentHandActions.getActions(GameState::Preflop).begin();
             i != ctx.personalContext.actions.currentHandActions.getActions(GameState::Preflop).end(); i++)
        {
            if (i->type == ActionType::Raise || i->type == ActionType::Allin)
            {
                playerRaises++;
            }
        }

        if (action.type == ActionType::Raise || action.type == ActionType::Allin)
        {

            if (playerRaises == 1 && ctx.commonContext.playersContext.raisersPositions.size() == 2)
            {
                myStatistics[nbPlayers].preflopStatistics.threeBets++;
            }

            if (playerRaises == 2 ||
                (playerRaises == 1 && ctx.commonContext.playersContext.raisersPositions.size() == 3))
            {
                myStatistics[nbPlayers].preflopStatistics.fourBets++;
            }
        }
        else
        {

            if (playerRaises == 1)
            {

                myStatistics[nbPlayers].preflopStatistics.callthreeBetsOpportunities++;

                if (action.type == ActionType::Call)
                {
                    myStatistics[nbPlayers].preflopStatistics.callthreeBets++;
                }
            }
        }
    }
}
void PlayerStatisticsUpdater::updateFlopStatistics(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    auto actions = ctx.personalContext.actions.currentHandActions.getActions(GameState::Flop);
    if (actions.empty())
        return;

    myStatistics[nbPlayers].flopStatistics.hands++;

    for (const auto& action : actions)
    {

        switch (action.type)
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
        if (action.type == ActionType::Raise && ctx.commonContext.playersContext.raisersPositions.size() > 1)
        {
            myStatistics[nbPlayers].flopStatistics.threeBets++;
        }

        // continuation bets
        if (ctx.commonContext.playersContext.preflopLastRaiser &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() == ctx.personalContext.id)
        {
            myStatistics[nbPlayers].flopStatistics.continuationBetsOpportunities++;
            if (action.type == ActionType::Bet)
            {
                myStatistics[nbPlayers].flopStatistics.continuationBets++;
            }
        }
    }
}
void PlayerStatisticsUpdater::updateTurnStatistics(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    auto actions = ctx.personalContext.actions.currentHandActions.getActions(GameState::Turn);
    if (actions.empty())
        return;

    myStatistics[nbPlayers].turnStatistics.hands++;

    for (const auto& action : actions)
    {
        switch (action.type)
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
        if (action.type == ActionType::Raise && ctx.commonContext.playersContext.raisersPositions.size() > 1)
        {
            myStatistics[nbPlayers].turnStatistics.threeBets++;
        }
    }
}
void PlayerStatisticsUpdater::updateRiverStatistics(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    auto actions = ctx.personalContext.actions.currentHandActions.getActions(GameState::River);
    if (actions.empty())
        return;

    myStatistics[nbPlayers].riverStatistics.hands++;

    for (const auto& action : actions)
    {
        switch (action.type)
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
        if (action.type == ActionType::Raise && ctx.commonContext.playersContext.raisersPositions.size() > 1)
        {
            myStatistics[nbPlayers].riverStatistics.threeBets++;
        }
    }
}

void PlayerStatisticsUpdater::ensureServicesInitialized() const
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

void PlayerStatisticsUpdater::loadStatistics(const std::string& playerName)
{
    ensureServicesInitialized();

    resetPlayerStatistics(); // reset stats to 0

    myStatistics = myServices->playersStatisticsStore().loadPlayerStatistics(playerName);
    if (myStatistics.empty())
    {
        myStatistics.fill(PlayerStatistics());
    }
}

} // namespace pkt::core::player
