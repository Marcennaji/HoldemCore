#include "RangeManager.h"

#include <core/engine/model/PlayerStatistics.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/Helpers.h>
#include <core/player/Player.h>
#include <core/player/RangePlausibilityChecker.h>
#include <core/player/strategy/CurrentHandContext.h>
#include "RangeRefiner.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

namespace pkt::core::player
{

using namespace std;

RangeManager::RangeManager(int playerId, IPlayersStatisticsStore* statsStore)
    : myPlayerId(playerId), myHand(nullptr), myStatsStore(statsStore)
{
    assert(myPlayerId >= 0);
}

void RangeManager::setEstimatedRange(const std::string& range)
{
    myEstimatedRange = range;
}

std::string RangeManager::getEstimatedRange() const
{
    return myEstimatedRange;
}

std::vector<std::string> RangeManager::getRangeAtomicValues(const std::string& ranges, const bool returnRange)
{
    std::vector<std::string> result;

    std::istringstream oss(ranges);
    std::string token;

    while (std::getline(oss, token, ','))
    {
        if (token.empty())
            continue;

        if (token.size() == 1 || token.size() > 4)
        {
            std::cerr << "getRangeAtomicValues invalid range: " << token << std::endl;
            return result;
        }

        const char* range = token.c_str();

        if (token.size() == 2)
        {
            handleExactPair(range, returnRange, result);
        }
        else if (token.size() == 3)
        {
            handleThreeCharRange(range, returnRange, result);
        }
        else if (token.size() == 4)
        {
            handleFourCharRange(range, returnRange, result);
        }
    }

    return result;
}

void RangeManager::handleExactPair(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    std::string s1(1, range[0]);
    std::string s2(1, range[1]);

    if (!returnRange)
    {
        result.push_back(s1 + 's' + s2 + 'd');
        result.push_back(s1 + 's' + s2 + 'h');
        result.push_back(s1 + 's' + s2 + 'c');
        result.push_back(s1 + 'd' + s2 + 'h');
        result.push_back(s1 + 'd' + s2 + 'c');
        result.push_back(s1 + 'c' + s2 + 'h');
    }
    else
    {
        result.push_back(s1 + s2);
    }
}

void RangeManager::handleThreeCharRange(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    std::string s1(1, range[0]);
    std::string s2(1, range[1]);

    if (range[2] == 's')
    {
        handleSuitedRange(s1, s2, returnRange, result);
    }
    else if (range[2] == 'o')
    {
        handleOffsuitedRange(s1, s2, returnRange, result);
    }
    else if (range[0] == range[1] && range[2] == '+')
    {
        handlePairAndAboveRange(range[0], returnRange, result);
    }
}

void RangeManager::handleFourCharRange(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    std::string s1(1, range[0]);
    char c = range[1];

    if (range[2] == 'o' && range[3] == '+')
    {
        handleOffsuitedAndAboveRange(s1, c, returnRange, result);
    }
    else if (range[2] == 's' && range[3] == '+')
    {
        handleSuitedAndAboveRange(s1, c, returnRange, result);
    }
    else if (range[0] == range[2])
    {
        handleExactPairRange(range, returnRange, result);
    }
    else
    {
        result.push_back(range); // Real hand, not a pair
    }
}

void RangeManager::handleSuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                     std::vector<std::string>& result)
{
    if (!returnRange)
    {
        result.push_back(s1 + 's' + s2 + 's');
        result.push_back(s1 + 'd' + s2 + 'd');
        result.push_back(s1 + 'h' + s2 + 'h');
        result.push_back(s1 + 'c' + s2 + 'c');
    }
    else
    {
        result.push_back(s1 + s2 + 's');
    }
}

void RangeManager::handleOffsuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                        std::vector<std::string>& result)
{
    if (!returnRange)
    {
        result.push_back(s1 + 's' + s2 + 'd');
        result.push_back(s1 + 's' + s2 + 'c');
        result.push_back(s1 + 's' + s2 + 'h');
        result.push_back(s1 + 'd' + s2 + 's');
        result.push_back(s1 + 'd' + s2 + 'c');
        result.push_back(s1 + 'd' + s2 + 'h');
        result.push_back(s1 + 'h' + s2 + 'd');
        result.push_back(s1 + 'h' + s2 + 'c');
        result.push_back(s1 + 'h' + s2 + 's');
        result.push_back(s1 + 'c' + s2 + 'd');
        result.push_back(s1 + 'c' + s2 + 's');
        result.push_back(s1 + 'c' + s2 + 'h');
    }
    else
    {
        result.push_back(s1 + s2 + 'o');
    }
}

void RangeManager::handlePairAndAboveRange(char c, const bool returnRange, std::vector<std::string>& result)
{
    while (c != 'X')
    {
        std::string s(1, c);

        if (!returnRange)
        {
            result.push_back(s + 's' + s + 'd');
            result.push_back(s + 's' + s + 'c');
            result.push_back(s + 's' + s + 'h');
            result.push_back(s + 'd' + s + 'c');
            result.push_back(s + 'd' + s + 'h');
            result.push_back(s + 'h' + s + 'c');
        }
        else
        {
            result.push_back(s + s);
        }

        c = incrementCardValue(c);
    }
}

void RangeManager::handleOffsuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                                std::vector<std::string>& result)
{
    while (c != s1[0])
    {
        std::string s2(1, c);

        if (!returnRange)
        {
            result.push_back(s1 + 's' + s2 + 'd');
            result.push_back(s1 + 's' + s2 + 'c');
            result.push_back(s1 + 's' + s2 + 'h');
            result.push_back(s1 + 'd' + s2 + 's');
            result.push_back(s1 + 'd' + s2 + 'c');
            result.push_back(s1 + 'd' + s2 + 'h');
            result.push_back(s1 + 'h' + s2 + 'd');
            result.push_back(s1 + 'h' + s2 + 'c');
            result.push_back(s1 + 'h' + s2 + 's');
            result.push_back(s1 + 'c' + s2 + 'd');
            result.push_back(s1 + 'c' + s2 + 's');
            result.push_back(s1 + 'c' + s2 + 'h');
        }
        else
        {
            result.push_back(s1 + s2 + 'o');
        }

        c = incrementCardValue(c);
    }
}

void RangeManager::handleSuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                             std::vector<std::string>& result)
{
    while (c != s1[0])
    {
        std::string s2(1, c);

        if (!returnRange)
        {
            result.push_back(s1 + 's' + s2 + 's');
            result.push_back(s1 + 'd' + s2 + 'd');
            result.push_back(s1 + 'h' + s2 + 'h');
            result.push_back(s1 + 'c' + s2 + 'c');
        }
        else
        {
            result.push_back(s1 + s2 + 's');
        }

        c = incrementCardValue(c);
    }
}

void RangeManager::handleExactPairRange(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    if (returnRange)
    {
        std::string s(1, range[0]);
        s += range[0];
        result.push_back(s);
    }
    else
    {
        result.push_back(range);
    }
}

char RangeManager::incrementCardValue(char c)
{

    switch (c)
    {
    case '2':
        return '3';
    case '3':
        return '4';
    case '4':
        return '5';
    case '5':
        return '6';
    case '6':
        return '7';
    case '7':
        return '8';
    case '8':
        return '9';
    case '9':
        return 'T';
    case 'T':
        return 'J';
    case 'J':
        return 'Q';
    case 'Q':
        return 'K';
    case 'K':
        return 'A';
    default:
        return 'X';
    }
}

void RangeManager::computeEstimatedPreflopRange(CurrentHandContext& ctx)
{
    using std::cout;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\t\testimated range for " << ctx.myID << " : ";
#endif

    PreflopStatistics preflop = ctx.myStatistics.getPreflopStatistics();
    const int nbPlayers = ctx.nbPlayers;

#ifdef LOG_POKER_EXEC
    std::cout << "  " << preflop.getVoluntaryPutMoneyInPot() << "/" << preflop.getPreflopRaise()
              << ", 3B: " << preflop.getPreflop3Bet() << ", 4B: " << preflop.getPreflop4Bet()
              << ", C3B: " << preflop.getPreflopCall3BetsFrequency() << ", pot odd: " << ctx.potOdd << " " << endl
              << "\t\t";
#endif

    // if the player was BB and has checked preflop, then he can have anything, except his approximative BB raising
    // range
    if (myHand->getPreflopRaisesNumber() == 0 && ctx.myPosition == BB)
    {

#ifdef LOG_POKER_EXEC
        cout << "any cards except ";
        if (preflop.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            cout << getStringRange(nbPlayers, preflop.getPreflopRaise() * 0.8);
        else
            cout << getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers) * 0.8);
        cout << endl;
#endif

        if (preflop.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            setEstimatedRange(
                deduceRange(ANY_CARDS_RANGE, getStringRange(ctx.nbPlayers, preflop.getPreflopRaise() * 0.8)));
        else
            setEstimatedRange(deduceRange(ANY_CARDS_RANGE,
                                          getStringRange(ctx.nbPlayers, getStandardRaisingRange(ctx.nbPlayers) * 0.8)));

        return;
    }

    string estimatedRange;

    // if the player is the last raiser :
    if (ctx.preflopLastRaiser && ctx.myID == ctx.preflopLastRaiser->getID())
        estimatedRange = computeEstimatedPreflopRangeFromLastRaiser(ctx);
    else
        estimatedRange = computeEstimatedPreflopRangeFromCaller(ctx);

#ifdef LOG_POKER_EXEC
    cout << " {" << estimatedRange << "}" << endl;
#endif

    setEstimatedRange(estimatedRange);
}

std::string RangeManager::computeEstimatedPreflopRangeFromLastRaiser(CurrentHandContext& ctx) const
{
    using std::cout;
    const int nbPlayers = ctx.nbPlayers;

    float range = 0;

#ifdef LOG_POKER_EXEC
    cout << " [ player is last raiser ] " << endl << "\t\t";
#endif

    PreflopStatistics preflopOpponent = ctx.preflopLastRaiser->getStatistics(nbPlayers).getPreflopStatistics();

    if (ctx.myStatistics.getPreflopStatistics().m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {
        if (myHand->getPreflopRaisesNumber() == 1)
        {
            range = preflopOpponent.getPreflopRaise();
        }
        else
        {
            // Assume the player has adapted their raising range to the previous raiser
            if (preflopOpponent.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            {
                if (myHand->getPreflopRaisesNumber() == 2)
                    range = preflopOpponent.getPreflopRaise() * 0.7;
                else if (myHand->getPreflopRaisesNumber() == 3)
                    range = preflopOpponent.getPreflop3Bet() * 0.7;
                else if (myHand->getPreflopRaisesNumber() > 3)
                    range = preflopOpponent.getPreflop4Bet() / (myHand->getPreflopRaisesNumber() / 2);
            }
            else
            {
                if (myHand->getPreflopRaisesNumber() == 2)
                    range = preflopOpponent.getPreflopRaise();
                else if (myHand->getPreflopRaisesNumber() == 3)
                    range = preflopOpponent.getPreflop3Bet();
                else if (myHand->getPreflopRaisesNumber() > 3)
                    range = preflopOpponent.getPreflop4Bet() / (myHand->getPreflopRaisesNumber() / 2);
            }
        }
    }
    else
    {
        range = getStandardRaisingRange(nbPlayers);

#ifdef LOG_POKER_EXEC
        cout << ", but not enough hands -> getting the standard range : " << range << endl << "\t\t";
#endif

        if (myHand->getPreflopRaisesNumber() == 2)
            range = range * 0.3;
        else if (myHand->getPreflopRaisesNumber() == 3)
            range = range * 0.2;
        else if (myHand->getPreflopRaisesNumber() > 3)
            range = range * 0.1;
    }

#ifdef LOG_POKER_EXEC
    cout << "range is " << range;
#endif

    if (nbPlayers > 3)
    {
        // Adjust range based on position
        if (ctx.preflopLastRaiser->getPosition() == UTG || ctx.preflopLastRaiser->getPosition() == UTG_PLUS_ONE ||
            ctx.preflopLastRaiser->getPosition() == UTG_PLUS_TWO)
            range = range * 0.9;
        else if (ctx.preflopLastRaiser->getPosition() == BUTTON || ctx.preflopLastRaiser->getPosition() == CUTOFF)
            range = range * 1.5;

#ifdef LOG_POKER_EXEC
        cout << ", position adjusted range is " << range << endl << "\t\t";
#endif
    }

    // Adjust range for loose/aggressive mode
    if (ctx.preflopLastRaiser->isInVeryLooseMode(nbPlayers))
    {
        if (range < 40)
        {
            range = 40;
#ifdef LOG_POKER_EXEC
            cout << "\t\toveragression detected, setting range to " << range << endl;
#endif
        }
    }

    // Add error margin
    range++;

    range = ceil(range);

    if (range < 1)
        range = 1;

    if (range > 100)
        range = 100;

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\testimated range is " << range << " % ";
#endif

    return getStringRange(nbPlayers, range);
}

string RangeManager::computeEstimatedPreflopRangeFromCaller(CurrentHandContext& ctx) const
{
    // the player is not the last raiser, but he has called a raise or limped in preflop

    using std::cout;
    bool isTopRange = true;
    std::vector<string> ranges;
    std::vector<float> rangesValues;
    const int lastRaiserID = myHand->getPreflopLastRaiserID();
    int opponentRaises = 0;
    int opponentCalls = 0;
    const int nbPlayers = myHand->getActivePlayerList()->size();
    float range = 0;

    for (std::vector<PlayerAction>::const_iterator i = ctx.myCurrentHandActions.getPreflopActions().begin();
         i != ctx.myCurrentHandActions.getPreflopActions().end(); i++)
    {
        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            opponentRaises++;
        else if (*i == PLAYER_ACTION_CALL)
            opponentCalls++;
    }

    float estimatedStartingRange = ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot();

    if (ctx.myStatistics.getPreflopStatistics().m_hands < MIN_HANDS_STATISTICS_ACCURATE)
    { // not enough hands, assume the opponent is an average tight player
        estimatedStartingRange = getStandardCallingRange(nbPlayers);
#ifdef LOG_POKER_EXEC
        cout << " [ not enough hands, getting the standard calling range ] ";
#endif
    }

#ifdef LOG_POKER_EXEC
    cout << " estimated starting range : " << estimatedStartingRange << endl << "\t\t";
    ;
#endif

    if (nbPlayers > 3)
    { // adjust roughly the range giving the player's position

        if (ctx.myPosition == UTG || ctx.myPosition == UTG_PLUS_ONE || ctx.myPosition == UTG_PLUS_TWO)
            estimatedStartingRange = estimatedStartingRange * 0.9;
        else if (ctx.myPosition == BUTTON || ctx.myPosition == CUTOFF)
            estimatedStartingRange = estimatedStartingRange * 1.4;
    }

    if (estimatedStartingRange > 100)
        estimatedStartingRange = 100;

#ifdef LOG_POKER_EXEC
    cout << ", position adjusted starting range : " << estimatedStartingRange << endl << "\t\t";
    ;
#endif

    // adjust roughly, given the pot odd the player had preflop
    const int potOdd = ctx.potOdd;

    if (potOdd > 70 && potOdd < 85)
        estimatedStartingRange = estimatedStartingRange * 0.7;
    else if (potOdd >= 85 && potOdd < 95)
        estimatedStartingRange = estimatedStartingRange * 0.5;
    else if (potOdd >= 95 && potOdd < 99)
        estimatedStartingRange = estimatedStartingRange * 0.3;
    else if (potOdd >= 99)
        estimatedStartingRange = estimatedStartingRange * 0.1;
    else if (potOdd <= 20 && myHand->getPreflopRaisesNumber() < 2)
        estimatedStartingRange = 40;

#ifdef LOG_POKER_EXEC
    cout << ", pot odd adjusted starting range : " << estimatedStartingRange << endl << "\t\t";
    ;
#endif

    range = estimatedStartingRange;

    if (myHand->getPreflopRaisesNumber() == 0 &&
        ctx.myStatistics.getPreflopStatistics().m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    { // limp

        if (myHand->getRunningPlayerList()->size() > 3)
            range = estimatedStartingRange -
                    ctx.myStatistics.getPreflopStatistics()
                        .getPreflopRaise(); // a hand suitable to call but not to raise ? or limp for deception
        else
            range = estimatedStartingRange;

        if (range < 5)
            range = 5;

#ifdef LOG_POKER_EXEC
        cout << ", limp range : " << range << endl << "\t\t";
        ;
#endif
    }
    else

        if (myHand->getPreflopRaisesNumber() == 1 &&
            ctx.myStatistics.getPreflopStatistics().m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {
        range = estimatedStartingRange -
                ctx.myStatistics.getPreflopStatistics().getPreflop3Bet(); // a hand suitable to call but not to 3-bet
        if (range < 1)
            range = 1;
#ifdef LOG_POKER_EXEC
        cout << ", single bet call range : " << range << endl << "\t\t";
        ;
#endif
        if (ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() -
                ctx.myStatistics.getPreflopStatistics().getPreflopRaise() >
            15)
        {
            // loose player
            range = range / 2;
#ifdef LOG_POKER_EXEC
            cout << ", loose player adjusted range : " << range << endl << "\t\t";
            ;
#endif
        }
    }
    else

        if (myHand->getPreflopRaisesNumber() == 2)
    {

        if (ctx.myStatistics.getPreflopStatistics().m_hands < MIN_HANDS_STATISTICS_ACCURATE)
        { // not enough hands, assume the opponent is an average tight player
            range = estimatedStartingRange / 3;
#ifdef LOG_POKER_EXEC
            cout << ", 3-bet call range : " << range;
#endif
        }
        else
        {

            if (opponentRaises == 1)
            {
                // if the player is being 3-betted
                range = ctx.myStatistics.getPreflopStatistics().getPreflopRaise() *
                        ctx.myStatistics.getPreflopStatistics().getPreflopCall3BetsFrequency() / 100;

                // assume that the player will adapt his calling range to the raiser's 3-bet range
                PreflopStatistics lastRaiserStats =
                    ctx.preflopLastRaiser->getStatistics(ctx.nbPlayers).getPreflopStatistics();

                if (range < lastRaiserStats.getPreflop3Bet() * 0.8)
                    range = lastRaiserStats.getPreflop3Bet() * 0.8;

#ifdef LOG_POKER_EXEC
                cout << ", 3-bet call range : " << range << endl << "\t\t";
                ;
#endif
            }
            else
            {
                // the player didn't raise the pot before, and there are already 2 raisers
                range = ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() / 3;
#ifdef LOG_POKER_EXEC
                cout << ", 3-bet cold-call range : "
                     << ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() << " / 3 = " << range
                     << endl
                     << "\t\t";
                ;
#endif
            }
        }
    }
    else

        if (myHand->getPreflopRaisesNumber() > 2)
    {

        if (ctx.myStatistics.getPreflopStatistics().m_hands < MIN_HANDS_STATISTICS_ACCURATE)
        { // not enough hands, assume the opponent is an average tight player
            range = estimatedStartingRange / 5;
#ifdef LOG_POKER_EXEC
            cout << ", 4-bet call range : " << range;
#endif
        }
        else
        {

            if (opponentRaises > 0)
            {
                // if the player is facing a 4-bet, after having bet
                range = (ctx.myStatistics.getPreflopStatistics().getPreflop3Bet() *
                         ctx.myStatistics.getPreflopStatistics().getPreflopCall3BetsFrequency() / 100);
#ifdef LOG_POKER_EXEC
                cout << ", 4-bet call range : " << range << endl << "\t\t";
                ;
#endif
            }
            else
            {
                // the player didn't raise the pot before, and there are already 3 raisers
                range = ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() / 6;
#ifdef LOG_POKER_EXEC
                cout << ", 4-bet cold-call range : " << range << endl << "\t\t";
                ;
#endif
            }
        }
    }

    if (myHand->getPreflopRaisesNumber() >= 2)
    {

        // adjust roughly, given the pot odd the player had preflop
        if (potOdd > 70 && potOdd < 85)
            range = range * 0.7;
        else if (potOdd >= 85 && potOdd < 95)
            range = range * 0.5;
        else if (potOdd >= 95 && potOdd < 99)
            range = range * 0.3;
        else if (potOdd >= 99)
            range = range * 0.1;
#ifdef LOG_POKER_EXEC
        if (potOdd > 70)
            cout << ", 3-bet or more : readjusting range with the pot odd (" << potOdd << ") : range is now " << range
                 << endl
                 << "\t\t";
        ;
#endif
    }

    // if the last raiser was being loose or agressive for 8 hands or so, adjust the range for the caller of a raise
    if (opponentRaises > 0 && lastRaiserID != -1 && myHand->getPreflopRaisesNumber() == 1)
    {

        if (ctx.preflopLastRaiser->isInVeryLooseMode(nbPlayers))
        {
            if (nbPlayers > 6 && range < 20)
            {
                range = 20;
            }
            else if (nbPlayers > 4 && range < 30)
            {
                range = 30;
            }
            else if (nbPlayers <= 4 && range < 40)
            {
                range = 40;
            }
#ifdef LOG_POKER_EXEC
            cout << "\t\toveragression detected from the raiser, setting calling range to " << range << endl;
#endif
        }
    }

    // add an error margin
    range++;

    if (range < 1)
        range = 1;

    if (range > 100)
        range = 100;

    if (potOdd < 75 && ctx.myStatistics.getPreflopStatistics().m_hands >= MIN_HANDS_STATISTICS_ACCURATE &&
        ctx.myStatistics.getPreflopStatistics().getVoluntaryPutMoneyInPot() -
                ctx.myStatistics.getPreflopStatistics().getPreflopRaise() <
            10 &&
        ctx.myStatistics.getPreflopStatistics().getPreflopRaise() > 5 && nbPlayers > 2)
    {

        if (opponentRaises == 0 && myHand->getPreflopRaisesNumber() == 0)
        {
            // the opponent limped
            ranges.push_back(",QJo,JTo,T9o,98o,87o,76o,65o,QTo,KJo,KTo,");
            rangesValues.push_back(9);
            ranges.push_back(SUITED_CONNECTORS + ",QTs,J9s,");
            rangesValues.push_back(SUITED_CONNECTORS_RANGE_VALUE + 0.6);
            ranges.push_back(LOW_PAIRS);
            rangesValues.push_back(LOW_PAIRS_RANGE_VALUE);
            ranges.push_back(LOW_SUITED_ACES);
            rangesValues.push_back(LOW_SUITED_ACES_RANGE_VALUE);
            ranges.push_back(LOW_OFFSUITED_ACES);
            rangesValues.push_back(LOW_OFFSUITED_ACES_RANGE_VALUE);
            ranges.push_back(HIGH_OFFSUITED_CONNECTORS);
            rangesValues.push_back(HIGH_OFFSUITED_CONNECTORS_RANGE_VALUE);
            ranges.push_back(SUITED_ONE_GAPED);
            rangesValues.push_back(SUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(OFFSUITED_ONE_GAPED);
            rangesValues.push_back(OFFSUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(SUITED_TWO_GAPED);
            rangesValues.push_back(SUITED_TWO_GAPED_RANGE_VALUE);
            ranges.push_back(SUITED_BROADWAYS);
            rangesValues.push_back(SUITED_BROADWAYS_RANGE_VALUE);
            isTopRange = false;
        }
        else if (opponentRaises == 0 && myHand->getPreflopRaisesNumber() == 1 &&
                 ctx.myStatistics.getPreflopStatistics().getPreflop3Bet() > 0)
        {
            // the opponent called a single standard bet
            ranges.push_back(",77,88,99,87s,98s,T9s,JTs,");
            rangesValues.push_back(2.6);
            ranges.push_back(LOW_PAIRS);
            rangesValues.push_back(LOW_PAIRS_RANGE_VALUE);
            ranges.push_back(HIGH_OFFSUITED_CONNECTORS);
            rangesValues.push_back(HIGH_OFFSUITED_CONNECTORS_RANGE_VALUE);
            ranges.push_back(",66,TT,JJ,");
            rangesValues.push_back(1.4);
            ranges.push_back(SUITED_BROADWAYS);
            rangesValues.push_back(SUITED_BROADWAYS_RANGE_VALUE);
            ranges.push_back(",87s,98s,T9s,");
            rangesValues.push_back(0.9);
            ranges.push_back(",AQo,AJo,KQo,");
            rangesValues.push_back(2.7);
            ranges.push_back(LOW_SUITED_ACES);
            rangesValues.push_back(LOW_SUITED_ACES_RANGE_VALUE);
            ranges.push_back(HIGH_PAIRS);
            rangesValues.push_back(HIGH_PAIRS_RANGE_VALUE); // hiding a strong hand is possible
            ranges.push_back("AKo");
            rangesValues.push_back(0.9);
            ranges.push_back(HIGH_SUITED_ACES);
            rangesValues.push_back(HIGH_SUITED_ACES_RANGE_VALUE);
            ranges.push_back(SUITED_ONE_GAPED);
            rangesValues.push_back(SUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(OFFSUITED_ONE_GAPED);
            rangesValues.push_back(OFFSUITED_ONE_GAPED_RANGE_VALUE);
            ranges.push_back(SUITED_TWO_GAPED);
            rangesValues.push_back(SUITED_TWO_GAPED_RANGE_VALUE);
            ranges.push_back(OFFSUITED_BROADWAYS);
            rangesValues.push_back(OFFSUITED_BROADWAYS_RANGE_VALUE);
            isTopRange = false;
        }
    }

    range = ceil(range);

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\testimated range is " << range << " % ";
#endif

    if (!isTopRange)
    {
#ifdef LOG_POKER_EXEC
        cout << " [ not a top range ] ";
#endif
        return getFilledRange(ranges, rangesValues, range);
    }
    else
        return getStringRange(nbPlayers, range);
}
string RangeManager::getFilledRange(std::vector<string>& ranges, std::vector<float>& rangesValues,
                                    const float rangeMax) const
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
        // there was not enough ranges to fill
#ifdef LOG_POKER_EXEC
        cout << endl << "\t\t[ warning : not enough ranges to fill " << rangeMax << "%, setting a top range ]" << endl;
#endif
        const int nbPlayers = myHand->getActivePlayerList()->size();
        return getStringRange(nbPlayers, rangeMax);
    }
    else
        return estimatedRange;
}
int RangeManager::getStandardRaisingRange(int nbPlayers) const
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
int RangeManager::getStandardCallingRange(int nbPlayers) const
{
    return getStandardRaisingRange(nbPlayers) + 5;
}

std::string RangeManager::getStringRange(int nbPlayers, int range)
{

    if (range > 100)
    { // should never happen, but...
        cout << "warning : bad range in getStringRange : " << range << endl;
        range = 100;
    }

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
void RangeManager::updateUnplausibleRangesGivenPreflopActions(CurrentHandContext& ctx)
{

    computeEstimatedPreflopRange(ctx);
    const string originalEstimatedRange = getEstimatedRange();

#ifdef LOG_POKER_EXEC
    std::cout << endl
              << "\tPlausible range on preflop for player " << ctx.myID << " :\t" << getEstimatedRange() << endl;
#endif

    const int nbPlayers = ctx.nbPlayers;

    PreflopStatistics preflop = ctx.myStatistics.getPreflopStatistics();

    // if no raise and the BB checks :
    if (ctx.myCurrentHandActions.getPreflopActions().back() == PLAYER_ACTION_CHECK)
    {

        if (preflop.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            setEstimatedRange(deduceRange(getEstimatedRange(), getStringRange(nbPlayers, preflop.getPreflopRaise())));
        else
            setEstimatedRange(
                deduceRange(getEstimatedRange(), getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers))));
    }

    if (getEstimatedRange() == "")
        setEstimatedRange(originalEstimatedRange);

#ifdef LOG_POKER_EXEC
    // logUnplausibleHands(GAME_STATE_PREFLOP);
#endif
}

void RangeManager::updateUnplausibleRangesGivenFlopActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on flop, before update :\t" << getEstimatedRange() << endl;
#endif

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on flop

    FlopStatistics flop = ctx.myStatistics.getFlopStatistics();

    PreflopStatistics preflop = ctx.myStatistics.getPreflopStatistics();

    if (ctx.myIsInVeryLooseMode)
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" << getEstimatedRange()
                  << endl;
#endif
        return;
    }

    vector<std::string> ranges = getRangeAtomicValues(getEstimatedRange());

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
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenFlopCall(r, ctx);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenFlopCheck(r, ctx);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenFlopRaise(r, ctx);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenFlopBet(r, ctx);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenFlopAllin(r, ctx);

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

    setEstimatedRange(deduceRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    // logUnplausibleHands(GAME_STATE_FLOP);
#endif
}

// purpose : remove some unplausible hands, who would normally be in the estimated preflop range
void RangeManager::updateUnplausibleRangesGivenTurnActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const PlayerStatistics& stats = ctx.myStatistics;
    const bool bHavePosition = ctx.myHavePosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on turn, before update :\t" << getEstimatedRange() << endl;
#endif

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on turn

    TurnStatistics turn = stats.getTurnStatistics();
    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (ctx.myIsInVeryLooseMode)
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t" << getEstimatedRange()
                  << endl;
#endif
        return;
    }

    vector<std::string> ranges = getRangeAtomicValues(getEstimatedRange());

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
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenTurnCall(r, ctx);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenTurnCheck(r, ctx);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenTurnRaise(r, ctx);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenTurnBet(r, ctx);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenTurnAllin(r, ctx);

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

    setEstimatedRange(deduceRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    // logUnplausibleHands(GAME_STATE_TURN);
#endif
}

// purpose : remove some unplausible hands, woul would normally be in the estimated preflop range
void RangeManager::updateUnplausibleRangesGivenRiverActions(CurrentHandContext& ctx)
{

    const int nbPlayers = ctx.nbPlayers;
    const PlayerStatistics& stats = ctx.myStatistics;
    const bool bHavePosition = ctx.myHavePosition;
    const string originalEstimatedRange = getEstimatedRange();
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on river, before update :\t" << getEstimatedRange() << endl;
#endif

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on river

    RiverStatistics river = stats.getRiverStatistics();

    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (ctx.myIsInVeryLooseMode)
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl << "\tSeems to be on very loose mode : estimated range is\t" << getEstimatedRange() << endl;
#endif
        return;
    }

    vector<std::string> ranges = getRangeAtomicValues(getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + ctx.stringBoard).c_str(), &r);

        bool removeHand = false;

        if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_CALL)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenRiverCall(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_CHECK)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenRiverCheck(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_RAISE)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenRiverRaise(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_BET)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenRiverBet(r, ctx);
        else if (ctx.myCurrentHandActions.getRiverActions().back() == PLAYER_ACTION_ALLIN)
            removeHand = RangePlausibilityChecker::isUnplausibleHandGivenRiverAllin(r, ctx);

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

    setEstimatedRange(deduceRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

    if (getEstimatedRange() == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    // logUnplausibleHands(GAME_STATE_RIVER);
#endif
}

std::string RangeManager::deduceRange(const std::string& originRanges, const std::string& rangesToSubstract,
                                      const std::string& board) const
{
    return RangeRefiner::deduceRange(originRanges, rangesToSubstract, board);
}

} // namespace pkt::core::player