#include "RangeManager.h"

#include <algorithm>
#include <core/engine/model/PlayerStatistics.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/Helpers.h>
#include <core/player/Player.h>
#include <core/player/RangePlausibilityChecker.h>
#include <core/player/strategy/CurrentHandContext.h>

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

std::vector<std::string> RangeManager::getRangeAtomicValues(std::string ranges, const bool returnRange) const
{

    vector<std::string> result;

    std::istringstream oss(ranges);
    std::string token;

    while (getline(oss, token, ','))
    {

        if (token.size() == 0)
            continue;

        if (token.size() == 1 || token.size() > 4)
        {
            std::cout << "getRangeAtomicValues invalid range : " << token << endl;
            return result;
        }

        const char* range = token.c_str();

        if (token.size() == 2)
        { // an exact pair, like 55 or AA

            string s1;
            s1 = range[0];
            string s2;
            s2 = range[1];

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
                result.push_back(s1 + s2);

            continue;
        }

        if (token.size() == 3)
        {

            if (range[0] != range[1] && range[2] == 's')
            { // range is an exact suited hand, like QJs

                string s1;
                s1 = range[0];
                string s2;
                s2 = range[1];

                if (!returnRange)
                {
                    result.push_back(s1 + 's' + s2 + 's');
                    result.push_back(s1 + 'd' + s2 + 'd');
                    result.push_back(s1 + 'h' + s2 + 'h');
                    result.push_back(s1 + 'c' + s2 + 'c');
                }
                else
                    result.push_back(s1 + s2 + 's');

                continue;
            }

            if (range[0] != range[1] && range[2] == 'o')
            { // range is an exact offsuited cards, like KTo

                string s1;
                s1 = range[0];
                string s2;
                s2 = range[1];

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
                    result.push_back(s1 + s2 + 'o');

                continue;
            }

            if (range[0] == range[1] && range[2] == '+')
            { // range is a pair and above, like 99+
                char c = range[0];

                while (c != 'X')
                {

                    string s;
                    s = c;

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
                        result.push_back(s + s);

                    // next value :
                    c = incrementCardValue(c);
                }
                continue;
            }
        }

        if (token.size() == 4)
        {

            if (range[0] != range[1] && range[2] == 'o' && range[3] == '+')
            {

                // range is offsuited and above, like AQo+

                string s1;
                s1 = range[0];
                char c = range[1];

                while (c != range[0])
                {

                    string s2;
                    s2 = c;

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
                        result.push_back(s1 + s2 + 'o');

                    // next value :
                    c = incrementCardValue(c);
                }
                continue;
            }
            if (range[0] != range[1] && range[2] == 's' && range[3] == '+')
            {

                // range is suited and above, like AQs+

                string s1;
                s1 = range[0];
                char c = range[1];

                while (c != range[0])
                {

                    string s2;
                    s2 = c;

                    if (!returnRange)
                    {
                        result.push_back(s1 + 's' + s2 + 's');
                        result.push_back(s1 + 'd' + s2 + 'd');
                        result.push_back(s1 + 'h' + s2 + 'h');
                        result.push_back(s1 + 'c' + s2 + 'c');
                    }
                    else
                        result.push_back(s1 + s2 + 's');

                    // next value :
                    c = incrementCardValue(c);
                }
                continue;
            }

            // if not a "suited/unsuited and above" range :

            if (range[0] == range[2])
            {
                // it's a pair, like JhJd

                if (returnRange)
                {
                    string s;
                    s += range[0];
                    s += range[0];
                    result.push_back(s);
                }
                else
                    result.push_back(token);

                continue;
            }

            // otherwise, it is a real hand but not a pair (like Ad2c) -> don't modify it
            result.push_back(token);
        }
    }

    return result;
}

char RangeManager::incrementCardValue(char c) const
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
                substractRange(ANY_CARDS_RANGE, getStringRange(ctx.nbPlayers, preflop.getPreflopRaise() * 0.8)));
        else
            setEstimatedRange(substractRange(
                ANY_CARDS_RANGE, getStringRange(ctx.nbPlayers, getStandardRaisingRange(ctx.nbPlayers) * 0.8)));

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

std::string RangeManager::substractRange(const std::string originRanges, const std::string rangesToSubstract,
                                         const std::string board)
{

    std::istringstream oss(originRanges);
    std::string singleOriginRange;
    string newRange;

    while (getline(oss, singleOriginRange, ','))
    {

        if (singleOriginRange.size() == 0)
            continue;

        // singleOriginRange may contain 1 range of any type, i.e : "AJo+"  "99"  "22+"  "87s" , or even a real hand
        // like "As3c"

        vector<std::string> cardsInOriginRange = getRangeAtomicValues(
            singleOriginRange); // split this range (if needed) into real cards, like ",AhJc,AsQc,....."

        bool keepOriginRange = true;

        for (vector<std::string>::const_iterator originHand = cardsInOriginRange.begin();
             originHand != cardsInOriginRange.end(); originHand++)
        {

            const string originCard1 = (*originHand).substr(0, 2);
            const string originCard2 = (*originHand).substr(2, 4);

            if (isCardsInRange(originCard1, originCard2, rangesToSubstract))
            {

                keepOriginRange = false; // at least one hand must be substracted from the singleOriginRange, so we must
                                         // replace this range by an other (smaller) range

                // check if this hand has been previously included in the new range, when processing an other range
                std::string::size_type pos = newRange.find(*originHand);
                if (pos != std::string::npos)
                {
#ifdef LOG_POKER_EXEC
                    std::cout << "removing previously included hand";
#endif
                    newRange = newRange.erase(pos, 4);
#ifdef LOG_POKER_EXEC
                    std::cout << "...new range is now " << newRange << endl;
#endif
                    continue;
                }

                // cout << endl << "must remove " << originCard1 << originCard2 << endl;

                vector<std::string> atomicRangesInSingleOriginRange = getRangeAtomicValues(singleOriginRange, true);
                // atomicRangesInSingleOriginRange will now contain the singleOriginRange ranges, but converted to
                // remove the + signs. It may also contain real hands, like 5h4h. purpose : we will try to keep as few
                // "real hands" as possible, for better display readability via the GUI

                for (vector<std::string>::const_iterator atomicOriginRange = atomicRangesInSingleOriginRange.begin();
                     atomicOriginRange != atomicRangesInSingleOriginRange.end(); atomicOriginRange++)
                {

                    // std::cout << "single origin atomic range is " << *atomicOriginRange << endl;

                    // if the "range" is in fact a real hand :
                    if (atomicOriginRange->size() == 4)
                    {

                        const string s1 = (*atomicOriginRange).substr(0, 2);
                        const string s2 = (*atomicOriginRange).substr(2, 4);
                        if (isCardsInRange(s1, s2, rangesToSubstract))
                            continue;
                        // delete hands that can't exist, given the board
                        if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                            continue;
                        if (newRange.find(*atomicOriginRange) == string::npos)
                            newRange += "," + (*atomicOriginRange); // don't put it twice in the new range

                        continue;
                    }

                    // process the real ranges like AQo AKo A5s 55 77 ....(i.e, atomic ranges, with no + or -)

                    if (originCard1.at(1) == originCard2.at(1))
                    {

                        // if we are processing a suited hand

                        string suitedRanges;
                        int nbSuitedRanges = 0;

                        vector<std::string> handsInAtomicRange = getRangeAtomicValues(*atomicOriginRange);

                        for (vector<std::string>::const_iterator i = handsInAtomicRange.begin();
                             i != handsInAtomicRange.end(); i++)
                        {

                            // std::cout << "hand in atomic range is " << *i << endl;

                            if (newRange.find(*i) != string::npos)
                                continue; // don't put it twice in the new range

                            const string s1 = (*i).substr(0, 2);
                            const string s2 = (*i).substr(2, 4);

                            if (isCardsInRange(s1, s2, rangesToSubstract))
                                continue;

                            // delete hands that can't exist, given the board
                            if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                                continue;

                            // std::cout << "we keep it" << endl;
                            nbSuitedRanges++;
                            suitedRanges += "," + (*i); // we keep this hand
                        }

                        if (nbSuitedRanges < 4)
                        {
                            newRange += suitedRanges; // put the real hands, like AhJh AsJs
                        }
                        else
                        {
                            // put a range like AJs, for better readability, instead of putting "AhJh AdJd AcJc AsJs"
                            if (newRange.find(*atomicOriginRange) == string::npos)
                                newRange += "," + (*atomicOriginRange); // don't put it twice in the new range
                        }
                        // cout << "new range is now " << newRange << endl;
                    }

                    else
                    {

                        // unsuited hands, including pairs

                        vector<std::string> handsInAtomicRange = getRangeAtomicValues(*atomicOriginRange);

                        for (vector<std::string>::const_iterator i = handsInAtomicRange.begin();
                             i != handsInAtomicRange.end(); i++)
                        {

                            // std::cout << "hand in atomic range is " << *i << endl;

                            const string s1 = (*i).substr(0, 2);
                            const string s2 = (*i).substr(2, 4);

                            if (isCardsInRange(s1, s2, rangesToSubstract))
                                continue;

                            if (newRange.find(*atomicOriginRange) != string::npos)
                                continue; // don't put it twice in the new range

                            // delete hands that can't exist, given the board
                            if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                                continue;

                            // std::cout << "we keep it" << endl;
                            newRange += "," + (*atomicOriginRange); // we keep this range
                        }
                    }
                }
            }
        }

        if (keepOriginRange)
        { // all hands in the origin range are kept
            newRange += ",";
            newRange += singleOriginRange;
        }
    }
    // unsigned pos;
    // while ((pos = newRange.find(",,")) != string::npos)
    // newRange = newRange.replace(pos, 2, ",");

    return newRange;
}

std::string RangeManager::getHandToRange(const std::string card1, const std::string card2) const
{

    // receive a hand like Ac Ks and translate it into a range like AKo

    std::stringstream result;

    if (card1.size() != 2 || card2.size() != 2)
    {
        cout << "RangeManager::getHandToRange invalid hand : " << card1 << " " << card2 << endl;
        return "";
    }

    if (card1.at(0) == card2.at(0))
        result << card2.at(0) << card2.at(0); // pair
    else if (card1.at(1) == card2.at(1))      // suited hand
        result << card1.at(0) << card2.at(0) << "s";
    else
        result << card1.at(0) << card2.at(0) << "o";

    return result.str();
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
            setEstimatedRange(
                substractRange(getEstimatedRange(), getStringRange(nbPlayers, preflop.getPreflopRaise())));
        else
            setEstimatedRange(
                substractRange(getEstimatedRange(), getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers))));
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

    setEstimatedRange(substractRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

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

    setEstimatedRange(substractRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

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

    setEstimatedRange(substractRange(getEstimatedRange(), unplausibleRanges, ctx.stringBoard));

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

} // namespace pkt::core::player