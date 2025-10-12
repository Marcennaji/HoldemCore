// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "DrawProbabilityCalculator.h"
#include "core/ports/HandEvaluationEngine.h"

namespace pkt::core::probability
{

// Static lookup tables for probability calculations
const int DrawProbabilityCalculator::outsOddsOneCard[] = {
    0,  2,  4,  6,  8,  11, /* 0 to 5 outs */
    13, 15, 17, 19, 21,     /* 6 to 10 outs */
    24, 26, 28, 30, 32,     /* 11 to 15 outs */
    34, 36, 39, 41, 43      /* 16 to 20 outs */
};

const int DrawProbabilityCalculator::outsOddsTwoCard[] = {
    0,  4,  8,  13, 17, 20, /* 0 to 5 outs */
    24, 28, 32, 35, 38,     /* 6 to 10 outs */
    42, 45, 48, 51, 54,     /* 11 to 15 outs */
    57, 60, 62, 65, 68      /* 16 to 20 outs */
};

int DrawProbabilityCalculator::getDrawingProbability(const pkt::core::PostFlopAnalysisFlags& postFlopAnalysis)
{
    if (!postFlopAnalysis.usesFirst && !postFlopAnalysis.usesSecond)
    {
        return 0;
    }

    int outs = 0;

    // do not count outs for straight or flush, if the board is paired
    if (!postFlopAnalysis.isFullHousePossible)
    {
        outs = postFlopAnalysis.straightOuts + postFlopAnalysis.flushOuts + postFlopAnalysis.betterOuts;
    }
    else
    {
        outs = postFlopAnalysis.betterOuts;
    }

    if (outs == 0)
    {
        return 0;
    }

    if (outs > 20)
    {
        outs = 20;
    }

    // if the last raiser is allin on flop : we must count our odds for the turn AND the river
    // TODO : this is not correct, as we must also take into account the other players actions, and their stacks
    /*if (currentHand->getCurrentRoundState() == Flop)
    {
        const int lastRaiserId = currentHand->getLastRaiserId();

        if (lastRaiserId != -1)
        {
            std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserId);
            const std::vector<PlayerAction>& actions = lastRaiser->getCurrentHandActions().getActions(GameState::Flop);

            for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
                 itAction++)
                if ((*itAction) == ActionType::Allin)
                    return outsOddsTwoCard[outs];
        }
    }*/

    return outsOddsOneCard[outs];
}

int DrawProbabilityCalculator::getImplicitOdd(const pkt::core::PostFlopAnalysisFlags& state)
{
    // TODO compute implicit odd according to opponent's profiles, stack sizes and actions in this hand
    int implicitOdd = 0;
    return implicitOdd;
}

bool DrawProbabilityCalculator::isDrawingProbOk(const PostFlopAnalysisFlags& postFlopState, int potOdd)
{
    int implicitOdd = getImplicitOdd(postFlopState);
    int drawingProb = getDrawingProbability(postFlopState);

    if (drawingProb > 0)
    {
        if (drawingProb + implicitOdd >= potOdd)
        {
            return true;
        }
    }
    return false;
}

} // namespace pkt::core::probability
