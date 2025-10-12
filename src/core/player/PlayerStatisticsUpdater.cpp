
#include "PlayerStatisticsUpdater.h"
#include "core/player/Player.h"
#include "core/player/strategy/CurrentHandContext.h"

namespace pkt::core::player
{
PlayerStatisticsUpdater::PlayerStatisticsUpdater(pkt::core::PlayersStatisticsStore& statisticsStore)
    : m_statisticsStore(statisticsStore)
{
}

const PlayerStatistics& PlayerStatisticsUpdater::getStatistics(const int nbPlayers) const
{
    int clamped = nbPlayers;
    if (clamped < 0)
        clamped = 0;
    if (clamped > MAX_NUMBER_OF_PLAYERS)
        clamped = MAX_NUMBER_OF_PLAYERS;
    return m_statistics[clamped];
}
void PlayerStatisticsUpdater::resetPlayerStatistics()
{

    for (int i = 0; i <= MAX_NUMBER_OF_PLAYERS; i++)
    {
        m_statistics[i].reset();
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

    m_statistics[nbPlayers].totalHands++;
    m_statistics[nbPlayers].preflopStatistics.hands++;

    for (const auto& action : actions)
    {
        switch (action.type)
        {
        case ActionType::Allin:
            m_statistics[nbPlayers].preflopStatistics.raises++;
            break;
        case ActionType::Raise:
            m_statistics[nbPlayers].preflopStatistics.raises++;
            break;
        case ActionType::Fold:
            m_statistics[nbPlayers].preflopStatistics.folds++;
            break;
        case ActionType::Check:
            m_statistics[nbPlayers].preflopStatistics.checks++;
            break;
        case ActionType::Call:
            m_statistics[nbPlayers].preflopStatistics.calls++;
            break;
        default:
            break;
        }

        m_statistics[nbPlayers].preflopStatistics.addLastAction(action); // keep track of the last 10 actions

        if (action.type == ActionType::Call && ctx.commonContext.playersContext.raisersPositions.size() == 0)
        { //
            m_statistics[nbPlayers].preflopStatistics.limps++;
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
                m_statistics[nbPlayers].preflopStatistics.threeBets++;
            }

            if (playerRaises == 2 ||
                (playerRaises == 1 && ctx.commonContext.playersContext.raisersPositions.size() == 3))
            {
                m_statistics[nbPlayers].preflopStatistics.fourBets++;
            }
        }
        else
        {

            if (playerRaises == 1)
            {

                m_statistics[nbPlayers].preflopStatistics.callthreeBetsOpportunities++;

                if (action.type == ActionType::Call)
                {
                    m_statistics[nbPlayers].preflopStatistics.callthreeBets++;
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

    m_statistics[nbPlayers].flopStatistics.hands++;

    for (const auto& action : actions)
    {

        switch (action.type)
        {
        case ActionType::Allin:
            m_statistics[nbPlayers].flopStatistics.raises++;
            break;
        case ActionType::Raise:
            m_statistics[nbPlayers].flopStatistics.raises++;
            break;
        case ActionType::Fold:
            m_statistics[nbPlayers].flopStatistics.folds++;
            break;
        case ActionType::Check:
            m_statistics[nbPlayers].flopStatistics.checks++;
            break;
        case ActionType::Call:
            m_statistics[nbPlayers].flopStatistics.calls++;
            break;
        case ActionType::Bet:
            m_statistics[nbPlayers].flopStatistics.bets++;
            break;
        default:
            break;
        }
        if (action.type == ActionType::Raise && ctx.commonContext.playersContext.raisersPositions.size() > 1)
        {
            m_statistics[nbPlayers].flopStatistics.threeBets++;
        }

        // continuation bets
        if (ctx.commonContext.playersContext.preflopLastRaiser &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() == ctx.personalContext.id)
        {
            m_statistics[nbPlayers].flopStatistics.continuationBetsOpportunities++;
            if (action.type == ActionType::Bet)
            {
                m_statistics[nbPlayers].flopStatistics.continuationBets++;
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

    m_statistics[nbPlayers].turnStatistics.hands++;

    for (const auto& action : actions)
    {
        switch (action.type)
        {
        case ActionType::Allin:
            m_statistics[nbPlayers].turnStatistics.raises++;
            break;
        case ActionType::Raise:
            m_statistics[nbPlayers].turnStatistics.raises++;
            break;
        case ActionType::Fold:
            m_statistics[nbPlayers].turnStatistics.folds++;
            break;
        case ActionType::Check:
            m_statistics[nbPlayers].turnStatistics.checks++;
            break;
        case ActionType::Call:
            m_statistics[nbPlayers].turnStatistics.calls++;
            break;
        case ActionType::Bet:
            m_statistics[nbPlayers].turnStatistics.bets++;
            break;
        default:
            break;
        }
        if (action.type == ActionType::Raise && ctx.commonContext.playersContext.raisersPositions.size() > 1)
        {
            m_statistics[nbPlayers].turnStatistics.threeBets++;
        }
    }
}
void PlayerStatisticsUpdater::updateRiverStatistics(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    auto actions = ctx.personalContext.actions.currentHandActions.getActions(GameState::River);
    if (actions.empty())
        return;

    m_statistics[nbPlayers].riverStatistics.hands++;

    for (const auto& action : actions)
    {
        switch (action.type)
        {
        case ActionType::Allin:
            m_statistics[nbPlayers].riverStatistics.raises++;
            break;
        case ActionType::Raise:
            m_statistics[nbPlayers].riverStatistics.raises++;
            break;
        case ActionType::Fold:
            m_statistics[nbPlayers].riverStatistics.folds++;
            break;
        case ActionType::Check:
            m_statistics[nbPlayers].riverStatistics.checks++;
            break;
        case ActionType::Call:
            m_statistics[nbPlayers].riverStatistics.calls++;
            break;
        case ActionType::Bet:
            m_statistics[nbPlayers].riverStatistics.bets++;
            break;
        default:
            break;
        }
        if (action.type == ActionType::Raise && ctx.commonContext.playersContext.raisersPositions.size() > 1)
        {
            m_statistics[nbPlayers].riverStatistics.threeBets++;
        }
    }
}

pkt::core::PlayersStatisticsStore& PlayerStatisticsUpdater::getPlayersStatisticsStore() const
{
    return m_statisticsStore;

}

void PlayerStatisticsUpdater::loadStatistics(const std::string& playerName)
{
    resetPlayerStatistics(); // reset stats to 0

    m_statistics = getPlayersStatisticsStore().loadPlayerStatistics(playerName);
    if (m_statistics.empty())
    {
        m_statistics.fill(PlayerStatistics());
    }
}

} // namespace pkt::core::player
