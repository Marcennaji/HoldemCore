#include "RangeEstimator.h"

#include <core/engine/model/PlayerStatistics.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/ILogger.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/Helpers.h>
#include <core/player/Player.h>
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

RangeEstimator::RangeEstimator(int playerId, IPlayersStatisticsStore* statsStore, ILogger* logger)
    : myPlayerId(playerId), myHand(nullptr), myStatsStore(statsStore), myLogger(logger)
{
    assert(myPlayerId >= 0);
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
        return estimatedRange;
}
int RangeEstimator::getStandardRaisingRange(int nbPlayers)
{

    if (nbPlayers == 2)
        return 39;
    else if (nbPlayers == 3)
        return 36;
    else if (nbPlayers == 4)
        return 33;
    else if (nbPlayers == 5)
        return 30;
    else if (nbPlayers == 6)
        return 27;
    else if (nbPlayers == 7)
        return 24;
    else if (nbPlayers == 8)
        return 21;
    else if (nbPlayers == 9)
        return 18;
    else
        return 15;
}
int RangeEstimator::getStandardCallingRange(int nbPlayers)
{
    return getStandardRaisingRange(nbPlayers) + 5;
}

std::string RangeEstimator::getStringRange(int nbPlayers, int range)
{

    if (range > 100)
        range = 100;

    if (nbPlayers == 2)
        return TOP_RANGE_2_PLAYERS[range];
    else if (nbPlayers == 3)
        return TOP_RANGE_3_PLAYERS[range];
    else if (nbPlayers == 4)
        return TOP_RANGE_4_PLAYERS[range];
    else
        return TOP_RANGE_MORE_4_PLAYERS[range];
}
// purpose : remove some unplausible hands (to my opponents eyes), given what I did preflop
void RangeEstimator::updateUnplausibleRangesGivenPreflopActions(CurrentHandContext& ctx)
{

    computeEstimatedPreflopRange(ctx);
    const string originalEstimatedRange = getEstimatedRange();

    myLogger->info("\tPlausible range on preflop for player " + std::to_string(ctx.myID) + " :\t" +
                   getEstimatedRange());

    const int nbPlayers = ctx.nbPlayers;

    PreflopStatistics preflop = ctx.myStatistics.getPreflopStatistics();

    // if no raise and the BB checks :
    if (ctx.myCurrentHandActions.getPreflopActions().back() == PLAYER_ACTION_CHECK)
    {

        if (preflop.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            setEstimatedRange(
                RangeRefiner::deduceRange(getEstimatedRange(), getStringRange(nbPlayers, preflop.getPreflopRaise())));
        else
            setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(),
                                                        getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers))));
    }

    if (getEstimatedRange() == "")
        setEstimatedRange(originalEstimatedRange);

    // logUnplausibleHands(GAME_STATE_PREFLOP);
}

void RangeEstimator::updateUnplausibleRangesGivenFlopActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    myLogger->info("\tPlausible range on flop, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on flop

    FlopStatistics flop = ctx.myStatistics.getFlopStatistics();

    PreflopStatistics preflop = ctx.myStatistics.getPreflopStatistics();

    if (ctx.myIsInVeryLooseMode)
    {
        myLogger->info("\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" + getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + ctx.stringBoard).c_str(), &r);

        bool removeHand = false;

        if (ctx.myCurrentHandActions.getFlopActions().empty())
        {
            // no flop action, so we can't remove any hand
            return;
        }

        PlayerAction myAction = ctx.myCurrentHandActions.getFlopActions().back();

        if (myAction == PLAYER_ACTION_CALL)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopCall(r, ctx);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(r, ctx);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(r, ctx);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(r, ctx);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenFlopAllin(r, ctx);

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
                unplausibleRanges += newUnplausibleRange;
        }
    }

    setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range

        myLogger->info("\tCan't remove all plausible ranges, keeping last one");

        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
        myLogger->info("\tRemoving unplausible ranges : " + unplausibleRanges);
    // logUnplausibleHands(GAME_STATE_FLOP);
}

// purpose : remove some unplausible hands, who would normally be in the estimated preflop range
void RangeEstimator::updateUnplausibleRangesGivenTurnActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const PlayerStatistics& stats = ctx.myStatistics;
    const bool bHavePosition = ctx.myHavePosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    myLogger->info("\tPlausible range on turn, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on turn

    TurnStatistics turn = stats.getTurnStatistics();
    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (ctx.myIsInVeryLooseMode)
    {
        myLogger->info("\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" + getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + ctx.stringBoard).c_str(), &r);

        bool removeHand = false;
        PlayerAction myAction = ctx.myCurrentHandActions.getTurnActions().back();

        if (myAction == PLAYER_ACTION_CALL)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(r, ctx);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnCheck(r, ctx);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(r, ctx);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(r, ctx);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(r, ctx);

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
                unplausibleRanges += newUnplausibleRange;
        }
    }

    setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range

        myLogger->info("\tCan't remove all plausible ranges, keeping last one");
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
        myLogger->info("\tRemoving unplausible ranges : " + unplausibleRanges);
    // logUnplausibleHands(GAME_STATE_TURN);
}

// purpose : remove some unplausible hands, woul would normally be in the estimated preflop range
void RangeEstimator::updateUnplausibleRangesGivenRiverActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const PlayerStatistics& stats = ctx.myStatistics;
    const bool bHavePosition = ctx.myHavePosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

    myLogger->info("\tPlausible range on river, before update :\t" + getEstimatedRange());

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on river

    RiverStatistics river = stats.getRiverStatistics();

    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (ctx.myIsInVeryLooseMode)
    {
        myLogger->info("\tSeems to be on very loose mode : estimated range is\t" + getEstimatedRange());
        return;
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + ctx.stringBoard).c_str(), &r);

        bool removeHand = false;

        if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_CALL)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_CHECK)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_RAISE)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_BET)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_ALLIN)
            removeHand = HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(r, ctx);

        if (removeHand)
        {

            string range = s1 + s2;
            string newUnplausibleRange = ",";
            newUnplausibleRange += range;
            newUnplausibleRange += ",";

            if (unplausibleRanges.find(newUnplausibleRange) == string::npos)
                unplausibleRanges += newUnplausibleRange;
        }
    }

    setEstimatedRange(RangeRefiner::deduceRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range

        myLogger->info("\tCan't remove all plausible ranges, keeping last one");
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

    if (unplausibleRanges != "")
        myLogger->info("\tRemoving unplausible ranges : " + unplausibleRanges);
    // logUnplausibleHands(GAME_STATE_RIVER);
}

void RangeEstimator::setHand(IHand* hand)
{
    myHand = hand;
    myPreflopRangeEstimator = make_unique<PreflopRangeEstimator>(myHand, myLogger, myPlayerId);
}

} // namespace pkt::core::player