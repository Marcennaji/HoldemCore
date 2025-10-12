#include "RangeEstimator.h"

#include <core/engine/model/Ranges.h>
#include <core/ports/PlayersStatisticsStore.h>
#include <core/player/Helpers.h>
#include <core/player/PlayerStatistics.h>
#include <core/player/range/HandPlausibilityChecker.h>
#include <core/player/strategy/CurrentHandContext.h>
#include "RangeParser.h"
#include "RangeRefiner.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

namespace pkt::core::player
{

using namespace std;

RangeEstimator::RangeEstimator(int playerId, pkt::core::Logger& logger, 
                               pkt::core::HandEvaluationEngine& handEvaluator)
    : m_playerId(playerId), m_logger(logger), m_handEvaluator(handEvaluator)
{
    assert(m_playerId >= 0);
     m_preflopRangeEstimator = make_unique<PreflopRangeEstimator>(m_playerId, logger);
}

void RangeEstimator::updateUnplausibleRanges(pkt::core::GameState state, const CurrentHandContext& ctx)
{
    switch (state)
    {
    case pkt::core::GameState::Preflop:
        updateUnplausibleRangesGivenPreflopActions(ctx);
        break;
    case pkt::core::GameState::Flop:
        updateUnplausibleRangesGivenFlopActions(ctx);
        break;
    case pkt::core::GameState::Turn:
        updateUnplausibleRangesGivenTurnActions(ctx);
        break;
    case pkt::core::GameState::River:
        updateUnplausibleRangesGivenRiverActions(ctx);
        break;
    default:
        break;
    }
}

void RangeEstimator::setEstimatedRange(const std::string& range)
{
    m_estimatedRange = range;
}

std::string RangeEstimator::getEstimatedRange() const
{
    return m_estimatedRange;
}

void RangeEstimator::computeEstimatedPreflopRange(const CurrentHandContext& ctx)
{
    setEstimatedRange(m_preflopRangeEstimator->computeEstimatedPreflopRange(ctx));
}

string RangeEstimator::getFilledRange(std::vector<string>& ranges, std::vector<float>& rangesValues,
                                      const float rangeMax, int nbPlayers)
{
    float remainingRange = rangeMax;
    string estimatedRange;

    for (int i = 0; i < ranges.size(); i++)
    {
        if (remainingRange > 0)
        {
            estimatedRange += ranges.at(i);
            remainingRange -= rangesValues.at(i);
        }
    }
    if (remainingRange > 0)
    {
        // there was not enough ranges to fill, so we set a top range
        return getStringRange(nbPlayers, rangeMax);
    }
    else
    {
        return estimatedRange;
    }
}
int RangeEstimator::getStandardRaisingRange(int nbPlayers)
{

    if (nbPlayers == 2)
    {
        return 39;
    }
    else if (nbPlayers == 3)
    {
        return 36;
    }
    else if (nbPlayers == 4)
    {
        return 33;
    }
    else if (nbPlayers == 5)
    {
        return 30;
    }
    else if (nbPlayers == 6)
    {
        return 27;
    }
    else if (nbPlayers == 7)
    {
        return 24;
    }
    else if (nbPlayers == 8)
    {
        return 21;
    }
    else if (nbPlayers == 9)
    {
        return 18;
    }
    else
    {
        return 15;
    }
}
int RangeEstimator::getStandardCallingRange(int nbPlayers)
{
    return getStandardRaisingRange(nbPlayers) + 5;
}

std::string RangeEstimator::getStringRange(int nbPlayers, int range)
{

    if (range > 100)
    {
        range = 100;
    }

    if (nbPlayers == 2)
    {
        return TOP_RANGE_2_PLAYERS[range];
    }
    else if (nbPlayers == 3)
    {
        return TOP_RANGE_3_PLAYERS[range];
    }
    else if (nbPlayers == 4)
    {
        return TOP_RANGE_4_PLAYERS[range];
    }
    else
    {
        return TOP_RANGE_MORE_4_PLAYERS[range];
    }
}
// purpose : remove some unplausible hands (to the opponents' eyes), given what the player did on preflop
void RangeEstimator::updateUnplausibleRangesGivenPreflopActions(const CurrentHandContext& ctx)
{
    computeEstimatedPreflopRange(ctx);
    const string originalEstimatedRange = getEstimatedRange();

    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;

    PreflopStatistics preflop = ctx.personalContext.statistics.preflopStatistics;

    // if no raise and the BB checks : guard against empty action list before accessing back()
    {
        const auto& preActions =
            ctx.personalContext.actions.currentHandActions.getActions(GameState::Preflop);
        if (!preActions.empty() && preActions.back().type == ActionType::Check)
    {

        if (preflop.hands >= MIN_HANDS_STATISTICS_ACCURATE)
        {
            setEstimatedRange(
                RangeRefiner::deduceRange(getEstimatedRange(), getStringRange(nbPlayers, preflop.getPreflopRaise())));
        }
        else
        {
            setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(),
                                                        getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers))));
        }
    }
    }

    if (getEstimatedRange() == "")
    {
        setEstimatedRange(originalEstimatedRange);
    }
    m_logger.info("\tPlausible range on preflop for player " + std::to_string(ctx.personalContext.id) +
                              " :\t" + getEstimatedRange());
}

void RangeEstimator::updateUnplausibleRangesGivenFlopActions(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    m_logger.verbose("\tPlausible range on flop, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on flop

    FlopStatistics flop = ctx.personalContext.statistics.flopStatistics;

    PreflopStatistics preflop = ctx.personalContext.statistics.preflopStatistics;

    if (ctx.personalContext.actions.isInVeryLooseMode)
    {
        m_logger.verbose("\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" +
                                     getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopAnalysisFlags postFlopFlags =
            m_handEvaluator.analyzeHand(stringHand, ctx.commonContext.stringBoard);

        bool removeHand = false;

        if (ctx.personalContext.actions.currentHandActions.getActions(GameState::Flop).empty())
        {
            // no flop action, so we can't remove any hand
            return;
        }

        ActionType m_action = ctx.personalContext.actions.currentHandActions.getActions(GameState::Flop).back().type;

        if (m_action == ActionType::Call)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopCall(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Check)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Raise)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Bet)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Allin)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopAllin(postFlopFlags, ctx);
        }

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
            {
                unplausibleRanges += newUnplausibleRange;
            }
        }
    }

    setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(), unplausibleRanges, ctx.commonContext.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range

        m_logger.verbose("\tCan't remove all plausible ranges, keeping last one");

        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
    {
        m_logger.verbose("\tRemoving unplausible hole cards : " + unplausibleRanges);
    }
}

// purpose : remove some unplausible hands, who would normally be in the estimated preflop range
void RangeEstimator::updateUnplausibleRangesGivenTurnActions(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerStatistics& stats = ctx.personalContext.statistics;
    const bool bHavePosition = ctx.personalContext.hasPosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    m_logger.verbose("\tPlausible range on turn, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on turn

    TurnStatistics turn = stats.turnStatistics;
    PreflopStatistics preflop = stats.preflopStatistics;

    if (ctx.personalContext.actions.isInVeryLooseMode)
    {
        m_logger.verbose("\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" +
                                     getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopAnalysisFlags postFlopFlags =
            m_handEvaluator.analyzeHand(stringHand, ctx.commonContext.stringBoard);

        bool removeHand = false;
        const auto& turnActions =
            ctx.personalContext.actions.currentHandActions.getActions(GameState::Turn);
        if (turnActions.empty())
        {
            // no turn action, so we can't remove any hand
            return;
        }
        ActionType m_action = turnActions.back().type;

        if (m_action == ActionType::Call)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Check)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnCheck(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Raise)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Bet)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(postFlopFlags, ctx);
        }
        else if (m_action == ActionType::Allin)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(postFlopFlags, ctx);
        }

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
            {
                unplausibleRanges += newUnplausibleRange;
            }
        }
    }

    setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(), unplausibleRanges, ctx.commonContext.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range

        m_logger.verbose("\tCan't remove all plausible ranges, keeping last one");
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
    {
        m_logger.verbose("\tRemoving unplausible hole cards : " + unplausibleRanges);
    }
}

// purpose : remove some unplausible hands, woul would normally be in the estimated preflop range
void RangeEstimator::updateUnplausibleRangesGivenRiverActions(const CurrentHandContext& ctx)
{
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerStatistics& stats = ctx.personalContext.statistics;
    const bool bHavePosition = ctx.personalContext.hasPosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    m_logger.verbose("\tPlausible range on river, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on river

    RiverStatistics river = stats.riverStatistics;

    PreflopStatistics preflop = stats.preflopStatistics;

    if (ctx.personalContext.actions.isInVeryLooseMode)
    {
        m_logger.verbose("\tSeems to be on very loose mode : estimated range is\t" + getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopAnalysisFlags postFlopFlags =
            m_handEvaluator.analyzeHand(stringHand, ctx.commonContext.stringBoard);

        bool removeHand = false;
        const auto& riverActions =
            ctx.personalContext.actions.currentHandActions.getActions(GameState::River);
        if (riverActions.empty())
        {
            // no river action, so we can't remove any hand
            return;
        }
        const ActionType lastRiverAction = riverActions.back().type;
        if (lastRiverAction == ActionType::Call)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(postFlopFlags, ctx);
        }
        else if (lastRiverAction == ActionType::Check)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(postFlopFlags, ctx);
        }
        else if (lastRiverAction == ActionType::Raise)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(postFlopFlags, ctx);
        }
        else if (lastRiverAction == ActionType::Bet)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(postFlopFlags, ctx);
        }
        else if (lastRiverAction == ActionType::Allin)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(postFlopFlags, ctx);
        }

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
            {
                unplausibleRanges += newUnplausibleRange;
            }
        }
    }

    setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(), unplausibleRanges, ctx.commonContext.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range

        m_logger.verbose("\tCan't remove all plausible ranges, keeping last one");
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
    {
        m_logger.verbose("\tRemoving unplausible hole cards : " + unplausibleRanges);
    }
}

} // namespace pkt::core::player