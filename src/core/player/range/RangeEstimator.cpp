#include "RangeEstimator.h"

#include <core/engine/model/PlayerStatistics.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/Helpers.h>
#include <core/player/Player.h>
#include <core/player/range/HandPlausibilityChecker.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/GlobalServices.h>
#include "RangeParser.h"
#include "RangeRefiner.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

namespace pkt::core::player
{

using namespace std;

RangeEstimator::RangeEstimator(int playerId) : myPlayerId(playerId)
{
    assert(myPlayerId >= 0);
    myPreflopRangeEstimator = make_unique<PreflopRangeEstimator>(myPlayerId);
}

void RangeEstimator::setEstimatedRange(const std::string& range)
{
    myEstimatedRange = range;
}

std::string RangeEstimator::getEstimatedRange() const
{
    return myEstimatedRange;
}

void RangeEstimator::computeEstimatedPreflopRange(CurrentHandContext& ctx)
{
    setEstimatedRange(myPreflopRangeEstimator->computeEstimatedPreflopRange(ctx));
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
// purpose : remove some unplausible hands (to my opponents eyes), given what I did preflop
void RangeEstimator::updateUnplausibleRangesGivenPreflopActions(CurrentHandContext& ctx)
{

    computeEstimatedPreflopRange(ctx);
    const string originalEstimatedRange = getEstimatedRange();

    GlobalServices::instance().logger()->verbose("\tPlausible range on preflop for player " +
                                                 std::to_string(ctx.perPlayerContext.myID) + " :\t" +
                                                 getEstimatedRange());

    const int nbPlayers = ctx.commonContext.nbPlayers;

    PreflopStatistics preflop = ctx.perPlayerContext.myStatistics.getPreflopStatistics();

    // if no raise and the BB checks :
    if (ctx.perPlayerContext.myCurrentHandActions.getPreflopActions().back() == ActionType::Check)
    {

        if (preflop.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
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

    if (getEstimatedRange() == "")
    {
        setEstimatedRange(originalEstimatedRange);
    }
}

void RangeEstimator::updateUnplausibleRangesGivenFlopActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    GlobalServices::instance().logger()->verbose("\tPlausible range on flop, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on flop

    FlopStatistics flop = ctx.perPlayerContext.myStatistics.getFlopStatistics();

    PreflopStatistics preflop = ctx.perPlayerContext.myStatistics.getPreflopStatistics();

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        GlobalServices::instance().logger()->verbose(
            "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" + getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopAnalysisFlags postFlopFlags =
            GlobalServices::instance().handEvaluationEngine()->analyzeHand(stringHand, ctx.commonContext.stringBoard);

        bool removeHand = false;

        if (ctx.perPlayerContext.myCurrentHandActions.getFlopActions().empty())
        {
            // no flop action, so we can't remove any hand
            return;
        }

        ActionType myAction = ctx.perPlayerContext.myCurrentHandActions.getFlopActions().back();

        if (myAction == ActionType::Call)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopCall(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Check)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Raise)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Bet)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Allin)
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

        GlobalServices::instance().logger()->verbose("\tCan't remove all plausible ranges, keeping last one");

        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
    {
        GlobalServices::instance().logger()->verbose("\tRemoving unplausible ranges : " + unplausibleRanges);
    }
}

// purpose : remove some unplausible hands, who would normally be in the estimated preflop range
void RangeEstimator::updateUnplausibleRangesGivenTurnActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const PlayerStatistics& stats = ctx.perPlayerContext.myStatistics;
    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    GlobalServices::instance().logger()->verbose("\tPlausible range on turn, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on turn

    TurnStatistics turn = stats.getTurnStatistics();
    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        GlobalServices::instance().logger()->verbose(
            "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" + getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopAnalysisFlags postFlopFlags =
            GlobalServices::instance().handEvaluationEngine()->analyzeHand(stringHand, ctx.commonContext.stringBoard);

        bool removeHand = false;
        ActionType myAction = ctx.perPlayerContext.myCurrentHandActions.getTurnActions().back();

        if (myAction == ActionType::Call)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Check)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnCheck(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Raise)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Bet)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(postFlopFlags, ctx);
        }
        else if (myAction == ActionType::Allin)
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

        GlobalServices::instance().logger()->verbose("\tCan't remove all plausible ranges, keeping last one");
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
    {
        GlobalServices::instance().logger()->verbose("\tRemoving unplausible ranges : " + unplausibleRanges);
    }
}

// purpose : remove some unplausible hands, woul would normally be in the estimated preflop range
void RangeEstimator::updateUnplausibleRangesGivenRiverActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.commonContext.nbPlayers;
    const PlayerStatistics& stats = ctx.perPlayerContext.myStatistics;
    const bool bHavePosition = ctx.perPlayerContext.myHavePosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    GlobalServices::instance().logger()->verbose("\tPlausible range on rivepostFlopFlags, before update :\t" +
                                                 getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on river

    RiverStatistics river = stats.getRiverStatistics();

    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (ctx.perPlayerContext.myIsInVeryLooseMode)
    {
        GlobalServices::instance().logger()->verbose("\tSeems to be on very loose mode : estimated range is\t" +
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
            GlobalServices::instance().handEvaluationEngine()->analyzeHand(stringHand, ctx.commonContext.stringBoard);

        bool removeHand = false;

        if (ctx.perPlayerContext.myCurrentHandActions.getRiverActions().back() == ActionType::Call)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(postFlopFlags, ctx);
        }
        else if (ctx.perPlayerContext.myCurrentHandActions.getRiverActions().back() == ActionType::Check)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(postFlopFlags, ctx);
        }
        else if (ctx.perPlayerContext.myCurrentHandActions.getRiverActions().back() == ActionType::Raise)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(postFlopFlags, ctx);
        }
        else if (ctx.perPlayerContext.myCurrentHandActions.getRiverActions().back() == ActionType::Bet)
        {
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(postFlopFlags, ctx);
        }
        else if (ctx.perPlayerContext.myCurrentHandActions.getRiverActions().back() == ActionType::Allin)
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

        GlobalServices::instance().logger()->verbose("\tCan't remove all plausible ranges, keeping last one");
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
    {
        GlobalServices::instance().logger()->verbose("\tRemoving unplausible ranges : " + unplausibleRanges);
    }
}

} // namespace pkt::core::player