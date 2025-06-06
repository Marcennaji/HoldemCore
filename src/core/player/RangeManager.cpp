#include "RangeManager.h"

#include <algorithm>
#include <core/engine/model/PlayerStatistics.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/IHand.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/Helpers.h>
#include <core/player/Player.h>

#include <iostream>
#include <sstream>

namespace pkt::core
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

// convert a range into a  list of real cards
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

            // if not a "suited/unsuited and above" range : range is a real hand, like Ad2c	or JhJd

            if (range[0] == range[2])
            {

                // it's a pair, like JhJd
                if (!returnRange)
                {
                    string s;
                    s += range[0];
                    s += range[0];
                    result.push_back(s);
                }
                else
                    result.push_back(token);
            }

            // real hand but not a pair (like Ad2c) : don't modify it
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
void RangeManager::computeEstimatedPreflopRange(Player& opponent, int nbPlayers, int lastRaiserID, int preflopRaises,
                                                const PreflopStatistics& lastRaiserStats,
                                                bool lastRaiserIsInVeryLooseMode)
{
    using std::cout;

#ifdef LOG_POKER_EXEC
    std::cout << endl
              << "\t\testimated range for " << (opponent.getID() == 0 ? "Human player" : opponent.getName()) << " : ";
#endif

    PreflopStatistics stats = opponent.getStatistics(nbPlayers).getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (stats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        opponent.getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        stats = opponent.getStatistics(nbPlayers + 1).getPreflopStatistics();

#ifdef LOG_POKER_EXEC
    std::cout << "  " << stats.getVoluntaryPutMoneyInPot() << "/" << stats.getPreflopRaise()
              << ", 3B: " << stats.getPreflop3Bet() << ", 4B: " << stats.getPreflop4Bet()
              << ", C3B: " << stats.getPreflopCall3BetsFrequency() << ", pot odd: " << opponent.getPreflopPotOdd()
              << " " << endl
              << "\t\t";
#endif

    // if the player was BB and has checked preflop, then he can have anything, except his approximative BB raising
    // range
    if (myHand->getPreflopRaisesNumber() == 0 && opponent.getPosition() == BB)
    {

#ifdef LOG_POKER_EXEC
        cout << "any cards except ";
        if (stats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            cout << getStringRange(nbPlayers, stats.getPreflopRaise() * 0.8);
        else
            cout << getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers) * 0.8);
        cout << endl;
#endif

        if (stats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            opponent.getRangeManager()->setEstimatedRange(
                opponent.substractRange(ANY_CARDS_RANGE, getStringRange(nbPlayers, stats.getPreflopRaise() * 0.8)));
        else
            opponent.getRangeManager()->setEstimatedRange(opponent.substractRange(
                ANY_CARDS_RANGE, getStringRange(nbPlayers, getStandardRaisingRange(nbPlayers) * 0.8)));

        return;
    }

    string estimatedRange;

    // if the player is the last raiser :
    if (opponent.getID() == lastRaiserID)
        estimatedRange = computeEstimatedPreflopRangeFromLastRaiser(opponent, stats, lastRaiserStats);
    else
        estimatedRange =
            computeEstimatedPreflopRangeFromCaller(opponent, stats, lastRaiserStats, lastRaiserIsInVeryLooseMode);

#ifdef LOG_POKER_EXEC
    cout << " {" << estimatedRange << "}" << endl;
#endif

    opponent.getRangeManager()->setEstimatedRange(estimatedRange);
}

std::string RangeManager::computeEstimatedPreflopRangeFromLastRaiser(const Player& opponent,
                                                                     const PreflopStatistics& opponentStats,
                                                                     const PreflopStatistics& previousRaiserStats) const
{
    using std::cout;
    const int nbPlayers = myHand->getActivePlayerList()->size();

    float range = 0;

#ifdef LOG_POKER_EXEC
    cout << " [ player is last raiser ] " << endl << "\t\t";
#endif

    if (opponentStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {
        if (myHand->getPreflopRaisesNumber() == 1)
        {
            range = opponentStats.getPreflopRaise();
        }
        else
        {
            // Assume the opponent has adapted their raising range to the previous raiser
            if (previousRaiserStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
            {
                if (myHand->getPreflopRaisesNumber() == 2)
                    range = previousRaiserStats.getPreflopRaise() * 0.7;
                else if (myHand->getPreflopRaisesNumber() == 3)
                    range = previousRaiserStats.getPreflop3Bet() * 0.7;
                else if (myHand->getPreflopRaisesNumber() > 3)
                    range = previousRaiserStats.getPreflop4Bet() / (myHand->getPreflopRaisesNumber() / 2);
            }
            else
            {
                if (myHand->getPreflopRaisesNumber() == 2)
                    range = opponentStats.getPreflopRaise();
                else if (myHand->getPreflopRaisesNumber() == 3)
                    range = opponentStats.getPreflop3Bet();
                else if (myHand->getPreflopRaisesNumber() > 3)
                    range = opponentStats.getPreflop4Bet() / (myHand->getPreflopRaisesNumber() / 2);
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
        if (opponent.getPosition() == UTG || opponent.getPosition() == UTG_PLUS_ONE ||
            opponent.getPosition() == UTG_PLUS_TWO)
            range = range * 0.9;
        else if (opponent.getPosition() == BUTTON || opponent.getPosition() == CUTOFF)
            range = range * 1.5;

#ifdef LOG_POKER_EXEC
        cout << ", position adjusted range is " << range << endl << "\t\t";
#endif
    }

    // Adjust range for loose/aggressive mode
    if (opponent.isInVeryLooseMode(nbPlayers))
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

string RangeManager::computeEstimatedPreflopRangeFromCaller(Player& caller, PreflopStatistics& callerStats,
                                                            const PreflopStatistics& lastRaiserStats,
                                                            bool lastRaiserIsInVeryLooseMode) const
{

    using std::cout;
    bool isTopRange = true;
    std::vector<string> ranges;
    std::vector<float> rangesValues;
    const int lastRaiserID = myHand->getPreflopLastRaiserID();
    int opponentRaises = 0;
    int opponentCalls = 0;
    const int nbPlayers = myHand->getActivePlayerList()->size();
    float range = 0;

    for (std::vector<PlayerAction>::const_iterator i = caller.getCurrentHandActions().getPreflopActions().begin();
         i != caller.getCurrentHandActions().getPreflopActions().end(); i++)
    {
        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            opponentRaises++;
        else if (*i == PLAYER_ACTION_CALL)
            opponentCalls++;
    }

    float estimatedStartingRange = callerStats.getVoluntaryPutMoneyInPot();

    if (callerStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE)
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

        if (caller.getPosition() == UTG || caller.getPosition() == UTG_PLUS_ONE || caller.getPosition() == UTG_PLUS_TWO)
            estimatedStartingRange = estimatedStartingRange * 0.9;
        else if (caller.getPosition() == BUTTON || caller.getPosition() == CUTOFF)
            estimatedStartingRange = estimatedStartingRange * 1.4;
    }

    if (estimatedStartingRange > 100)
        estimatedStartingRange = 100;

#ifdef LOG_POKER_EXEC
    cout << ", position adjusted starting range : " << estimatedStartingRange << endl << "\t\t";
    ;
#endif

    // adjust roughly, given the pot odd the player had preflop
    const int potOdd = caller.getPreflopPotOdd();

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

    if (myHand->getPreflopRaisesNumber() == 0 && callerStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    { // limp

        if (myHand->getRunningPlayerList()->size() > 3)
            range = estimatedStartingRange -
                    callerStats.getPreflopRaise(); // a hand suitable to call but not to raise ? or limp for deception
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

        if (myHand->getPreflopRaisesNumber() == 1 && callerStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE)
    {
        range = estimatedStartingRange - callerStats.getPreflop3Bet(); // a hand suitable to call but not to 3-bet
        if (range < 1)
            range = 1;
#ifdef LOG_POKER_EXEC
        cout << ", single bet call range : " << range << endl << "\t\t";
        ;
#endif
        if (callerStats.getVoluntaryPutMoneyInPot() - callerStats.getPreflopRaise() > 15)
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

        if (callerStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE)
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
                range = callerStats.getPreflopRaise() * callerStats.getPreflopCall3BetsFrequency() / 100;

                // assume that the player will adapt his calling range to the raiser's 3-bet range

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
                range = callerStats.getVoluntaryPutMoneyInPot() / 3;
#ifdef LOG_POKER_EXEC
                cout << ", 3-bet cold-call range : " << callerStats.getVoluntaryPutMoneyInPot() << " / 3 = " << range
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

        if (callerStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE)
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
                range = (callerStats.getPreflop3Bet() * callerStats.getPreflopCall3BetsFrequency() / 100);
#ifdef LOG_POKER_EXEC
                cout << ", 4-bet call range : " << range << endl << "\t\t";
                ;
#endif
            }
            else
            {
                // the player didn't raise the pot before, and there are already 3 raisers
                range = callerStats.getVoluntaryPutMoneyInPot() / 6;
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

        if (lastRaiserIsInVeryLooseMode)
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

    if (potOdd < 75 && callerStats.m_hands >= MIN_HANDS_STATISTICS_ACCURATE &&
        callerStats.getVoluntaryPutMoneyInPot() - callerStats.getPreflopRaise() < 10 &&
        callerStats.getPreflopRaise() > 5 && nbPlayers > 2)
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
        else if (opponentRaises == 0 && myHand->getPreflopRaisesNumber() == 1 && callerStats.getPreflop3Bet() > 0)
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

} // namespace pkt::core