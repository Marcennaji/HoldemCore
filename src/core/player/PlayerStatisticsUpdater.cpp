
#include "PlayerStatisticsUpdater.h"
#include "core/player/Player.h"
#include "core/player/strategy/CurrentHandContext.h"

namespace pkt::core::player
{
PlayerStatisticsUpdater::PlayerStatisticsUpdater(pkt::core::PlayersStatisticsStore& statisticsStore)
    : m_statisticsStore(statisticsStore), m_preflopCounted(false), m_flopCounted(false), m_turnCounted(false),
      m_riverCounted(false)
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

void PlayerStatisticsUpdater::resetHandCountingFlags()
{
    m_preflopCounted = false;
    m_flopCounted = false;
    m_turnCounted = false;
    m_riverCounted = false;
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

    // Only increment hands counter once per street, not on every action
    if (!m_preflopCounted)
    {
        m_statistics[nbPlayers].totalHands++;
        m_statistics[nbPlayers].preflopStatistics.hands++;
        m_preflopCounted = true;
    }

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

    // Only increment hands counter once per street, not on every action
    if (!m_flopCounted)
    {
        m_statistics[nbPlayers].flopStatistics.hands++;
        m_flopCounted = true;

        // Continuation bet opportunity: check once per street if player was preflop raiser
        if (ctx.commonContext.playersContext.preflopLastRaiser &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() == ctx.personalContext.id)
        {
            m_statistics[nbPlayers].flopStatistics.continuationBetsOpportunities++;
        }
    }

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

        // Continuation bet: increment only when we actually bet
        if (ctx.commonContext.playersContext.preflopLastRaiser &&
            ctx.commonContext.playersContext.preflopLastRaiser->getId() == ctx.personalContext.id &&
            action.type == ActionType::Bet)
        {
            m_statistics[nbPlayers].flopStatistics.continuationBets++;
        }
    }
}
void PlayerStatisticsUpdater::updateTurnStatistics(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    auto actions = ctx.personalContext.actions.currentHandActions.getActions(GameState::Turn);
    if (actions.empty())
        return;

    // Only increment hands counter once per street, not on every action
    if (!m_turnCounted)
    {
        m_statistics[nbPlayers].turnStatistics.hands++;
        m_turnCounted = true;
    }

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

    // Only increment hands counter once per street, not on every action
    if (!m_riverCounted)
    {
        m_statistics[nbPlayers].riverStatistics.hands++;
        m_riverCounted = true;
    }

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

void PlayerStatisticsUpdater::loadStatistics(const std::string& strategyName)
{
    resetPlayerStatistics(); // reset stats to 0

    m_statistics = getPlayersStatisticsStore().loadPlayerStatistics(strategyName);
    if (m_statistics.empty())
    {
        m_statistics.fill(PlayerStatistics());
    }

    // Initialize last saved to match loaded values (no delta yet)
    m_lastSavedStatistics = m_statistics;
}

PlayerStatistics PlayerStatisticsUpdater::getStatisticsDeltaAndUpdateBaseline(const int nbPlayers)
{
    int clamped = nbPlayers;
    if (clamped < 0)
        clamped = 0;
    if (clamped > MAX_NUMBER_OF_PLAYERS)
        clamped = MAX_NUMBER_OF_PLAYERS;

    // Calculate delta by subtracting last saved from current
    PlayerStatistics delta;
    const auto& current = m_statistics[clamped];
    const auto& lastSaved = m_lastSavedStatistics[clamped];

    delta.totalHands = current.totalHands - lastSaved.totalHands;

    // Preflop deltas
    delta.preflopStatistics.hands = current.preflopStatistics.hands - lastSaved.preflopStatistics.hands;
    delta.preflopStatistics.checks = current.preflopStatistics.checks - lastSaved.preflopStatistics.checks;
    delta.preflopStatistics.calls = current.preflopStatistics.calls - lastSaved.preflopStatistics.calls;
    delta.preflopStatistics.raises = current.preflopStatistics.raises - lastSaved.preflopStatistics.raises;
    delta.preflopStatistics.threeBets = current.preflopStatistics.threeBets - lastSaved.preflopStatistics.threeBets;
    delta.preflopStatistics.fourBets = current.preflopStatistics.fourBets - lastSaved.preflopStatistics.fourBets;
    delta.preflopStatistics.folds = current.preflopStatistics.folds - lastSaved.preflopStatistics.folds;
    delta.preflopStatistics.limps = current.preflopStatistics.limps - lastSaved.preflopStatistics.limps;
    delta.preflopStatistics.callthreeBets =
        current.preflopStatistics.callthreeBets - lastSaved.preflopStatistics.callthreeBets;
    delta.preflopStatistics.callthreeBetsOpportunities =
        current.preflopStatistics.callthreeBetsOpportunities - lastSaved.preflopStatistics.callthreeBetsOpportunities;

    // Flop deltas
    delta.flopStatistics.hands = current.flopStatistics.hands - lastSaved.flopStatistics.hands;
    delta.flopStatistics.checks = current.flopStatistics.checks - lastSaved.flopStatistics.checks;
    delta.flopStatistics.bets = current.flopStatistics.bets - lastSaved.flopStatistics.bets;
    delta.flopStatistics.calls = current.flopStatistics.calls - lastSaved.flopStatistics.calls;
    delta.flopStatistics.raises = current.flopStatistics.raises - lastSaved.flopStatistics.raises;
    delta.flopStatistics.folds = current.flopStatistics.folds - lastSaved.flopStatistics.folds;
    delta.flopStatistics.continuationBets =
        current.flopStatistics.continuationBets - lastSaved.flopStatistics.continuationBets;
    delta.flopStatistics.continuationBetsOpportunities =
        current.flopStatistics.continuationBetsOpportunities - lastSaved.flopStatistics.continuationBetsOpportunities;

    // Turn deltas
    delta.turnStatistics.hands = current.turnStatistics.hands - lastSaved.turnStatistics.hands;
    delta.turnStatistics.checks = current.turnStatistics.checks - lastSaved.turnStatistics.checks;
    delta.turnStatistics.bets = current.turnStatistics.bets - lastSaved.turnStatistics.bets;
    delta.turnStatistics.calls = current.turnStatistics.calls - lastSaved.turnStatistics.calls;
    delta.turnStatistics.raises = current.turnStatistics.raises - lastSaved.turnStatistics.raises;
    delta.turnStatistics.folds = current.turnStatistics.folds - lastSaved.turnStatistics.folds;

    // River deltas
    delta.riverStatistics.hands = current.riverStatistics.hands - lastSaved.riverStatistics.hands;
    delta.riverStatistics.checks = current.riverStatistics.checks - lastSaved.riverStatistics.checks;
    delta.riverStatistics.bets = current.riverStatistics.bets - lastSaved.riverStatistics.bets;
    delta.riverStatistics.calls = current.riverStatistics.calls - lastSaved.riverStatistics.calls;
    delta.riverStatistics.raises = current.riverStatistics.raises - lastSaved.riverStatistics.raises;
    delta.riverStatistics.folds = current.riverStatistics.folds - lastSaved.riverStatistics.folds;

    // Update baseline for next save
    m_lastSavedStatistics[clamped] = m_statistics[clamped];

    return delta;
}

} // namespace pkt::core::player
