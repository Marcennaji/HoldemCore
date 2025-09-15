// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PlayerStatistics.h"

namespace pkt::core::player
{

const int PreflopStatistics::LAST_ACTIONS_STACK_SIZE = 10;

void PlayerStatistics::reset()
{
    totalHands = 0;
    preflopStatistics.reset();
    flopStatistics.reset();
    turnStatistics.reset();
    riverStatistics.reset();
}

float PlayerStatistics::getWentToShowDown() const
{

    if ((riverStatistics.hands - riverStatistics.folds) == 0)
    {
        return 0;
    }

    return ((riverStatistics.hands - riverStatistics.folds) * 100) / (flopStatistics.hands);
}

void PreflopStatistics::addLastAction(PlayerAction p)
{
    m_lastActions.push_back(p);
    if (m_lastActions.size() > LAST_ACTIONS_STACK_SIZE)
    {
        m_lastActions.pop_front();
    }
}
int PreflopStatistics::getLastActionsNumber(ActionType p) const
{

    int result = 0;
    for (int i = 0; i < m_lastActions.size(); i++)
    {
        if (m_lastActions.at(i).type == p)
        {
            result++;
        }
    }

    return result;
}
void PreflopStatistics::reset()
{

    hands = 0;
    folds = 0;
    checks = 0;
    calls = 0;
    raises = 0;
    limps = 0;
    threeBets = 0;
    callthreeBets = 0;
    callthreeBetsOpportunities = 0;
    fourBets = 0;
}
float PreflopStatistics::getVoluntaryPutMoneyInPot() const
{

    if (hands == 0 || (raises + calls == 0))
    {
        return 0;
    }

    float f = (float) ((raises + calls) * 100) / (float) hands;

    return (f > 100 ? 100 : f);
}

float PreflopStatistics::getPreflopRaise() const
{

    if (hands == 0 || raises == 0)
    {
        return 0;
    }

    float f = (float) ((raises * 100) / (float) hands);

    return (f > 100 ? 100 : f);
}
float PreflopStatistics::getPreflop3Bet() const
{

    if (hands == 0 || threeBets == 0)
    {
        return 0;
    }

    float f = (float) ((threeBets * 100) / (float) hands);

    return (f > 100 ? 100 : f);
}
float PreflopStatistics::getPreflop4Bet() const
{

    if (hands == 0 || fourBets == 0)
    {
        return 0;
    }

    float f = (float) ((fourBets * 100) / (float) hands);

    return (f > 100 ? 100 : f);
}
float PreflopStatistics::getPreflopCallthreeBetsFrequency() const
{

    if (callthreeBetsOpportunities == 0)
    {
        return 0;
    }

    float f = (float) (callthreeBets) / (float) (callthreeBetsOpportunities) * 100;

    return (f > 100 ? 100 : f);
}

void FlopStatistics::reset()
{

    hands = 0;
    folds = 0;
    checks = 0;
    bets = 0;
    calls = 0;
    raises = 0;
    threeBets = 0;
    fourBets = 0;
    continuationBetsOpportunities = 0;
    continuationBets = 0;
}
float FlopStatistics::getAgressionFactor() const
{

    if (raises + bets == 0 || calls == 0)
    {
        return 0;
    }

    return (float) (raises + bets) / (float) calls;
}

float FlopStatistics::getAgressionFrequency() const
{

    if ((float) (raises + bets + calls + checks + folds) == 0.0)
    {
        return 0.0;
    }

    if ((float) (raises + bets) == 0.0)
    {
        return 0.0;
    }

    return (float) (raises + bets) / (float) (raises + bets + calls + checks + folds) * 100;
}

float FlopStatistics::getContinuationBetFrequency() const
{

    if (continuationBetsOpportunities == 0)
    {
        return 0;
    }

    return (float) (continuationBets) / (float) (continuationBetsOpportunities) * 100;
}

void TurnStatistics::reset()
{

    hands = 0;
    folds = 0;
    checks = 0;
    bets = 0;
    calls = 0;
    raises = 0;
    threeBets = 0;
    fourBets = 0;
}

float TurnStatistics::getAgressionFrequency() const
{

    if ((float) (raises + bets + calls + checks + folds) == 0.0)
    {
        return 0.0;
    }

    if ((float) (raises + bets) == 0.0)
    {
        return 0.0;
    }

    return (float) (raises + bets) / (float) (raises + bets + calls + checks + folds) * 100;
}

float TurnStatistics::getAgressionFactor() const
{

    if (raises + bets == 0 || calls == 0)
    {
        return 0;
    }

    return (float) (raises + bets) / (float) calls;
}
void RiverStatistics::reset()
{

    hands = 0;
    folds = 0;
    checks = 0;
    bets = 0;
    calls = 0;
    raises = 0;
    threeBets = 0;
    fourBets = 0;
}
float RiverStatistics::getAgressionFrequency() const
{

    if ((float) (raises + bets + calls + checks + folds) == 0.0)
    {
        return 0.0;
    }

    if ((float) (raises + bets) == 0.0)
    {
        return 0.0;
    }

    return (float) (raises + bets) / (float) (raises + bets + calls + checks + folds) * 100;
}
float RiverStatistics::getAgressionFactor() const
{

    if (raises + bets == 0 || calls == 0)
    {
        return 0;
    }

    return (float) (raises + bets) / (float) calls;
}

} // namespace pkt::core::player
