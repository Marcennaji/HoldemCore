
#include "PreflopRangeCalculator.h"
#include "CurrentHandContext.h"
#include "core/engine/Randomizer.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"

namespace pkt::core::player
{
using namespace std;

void PreflopRangeCalculator::initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
{

    const float step = (float) (utgHeadsUpRange - utgFullTableRange) / 8;

    // values are % best hands

    UTG_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    UTG_STARTING_RANGE[2] = utgHeadsUpRange;
    UTG_STARTING_RANGE[3] = utgHeadsUpRange - step;
    UTG_STARTING_RANGE[4] = utgHeadsUpRange - (2 * step);
    UTG_STARTING_RANGE[5] = utgHeadsUpRange - (3 * step);
    UTG_STARTING_RANGE[6] = utgHeadsUpRange - (4 * step);
    UTG_STARTING_RANGE[7] = utgFullTableRange + (3 * step);
    UTG_STARTING_RANGE[8] = utgFullTableRange + (2 * step);
    UTG_STARTING_RANGE[9] = utgFullTableRange + step;
    UTG_STARTING_RANGE[10] = utgFullTableRange;

    assert(UTG_STARTING_RANGE[7] < UTG_STARTING_RANGE[6]);

    // we have the UTG starting ranges. Now, deduce the starting ranges for other positions :

    UTG_PLUS_ONE_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < UTG_PLUS_ONE_STARTING_RANGE.size(); i++)
    {
        UTG_PLUS_ONE_STARTING_RANGE[i] = min(50, UTG_STARTING_RANGE[i] + 1);
    }

    UTG_PLUS_TWO_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < UTG_PLUS_TWO_STARTING_RANGE.size(); i++)
    {
        UTG_PLUS_TWO_STARTING_RANGE[i] = min(50, UTG_PLUS_ONE_STARTING_RANGE[i] + 1);
    }

    MIDDLE_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < MIDDLE_STARTING_RANGE.size(); i++)
    {
        MIDDLE_STARTING_RANGE[i] = min(50, UTG_PLUS_TWO_STARTING_RANGE[i] + 1);
    }

    MIDDLE_PLUS_ONE_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < MIDDLE_PLUS_ONE_STARTING_RANGE.size(); i++)
    {
        MIDDLE_PLUS_ONE_STARTING_RANGE[i] = min(50, MIDDLE_STARTING_RANGE[i] + 1);
    }

    LATE_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < LATE_STARTING_RANGE.size(); i++)
    {
        LATE_STARTING_RANGE[i] = min(50, MIDDLE_PLUS_ONE_STARTING_RANGE[i] + 1);
    }

    CUTOFF_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < CUTOFF_STARTING_RANGE.size(); i++)
    {
        CUTOFF_STARTING_RANGE[i] = min(50, LATE_STARTING_RANGE[i] + 1);
    }

    BUTTON_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < BUTTON_STARTING_RANGE.size(); i++)
    {
        BUTTON_STARTING_RANGE[i] = min(50, CUTOFF_STARTING_RANGE[i] + 1);
    }

    SB_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < SB_STARTING_RANGE.size(); i++)
    {
        SB_STARTING_RANGE[i] = CUTOFF_STARTING_RANGE[i];
    }

    BB_STARTING_RANGE.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < BB_STARTING_RANGE.size(); i++)
    {
        BB_STARTING_RANGE[i] = SB_STARTING_RANGE[i] + 1;
    }
}

float PreflopRangeCalculator::calculatePreflopCallingRange(CurrentHandContext& context, bool deterministic) const
{
    const int nbRaises = context.preflopRaisesNumber;
    const int nbCalls = context.preflopCallsNumber;
    const int nbPlayers = context.nbPlayers;
    const PlayerPosition myPosition = context.myPosition;
    const int potOdd = context.potOdd;
    const int myCash = context.myCash;
    const bool isPreflopBigBet = context.isPreflopBigBet;
    const int mySet = context.mySet;
    const int smallBlind = context.smallBlind;
    const int myM = context.myM;

    float callingRange = getRange(myPosition, nbPlayers);

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\tPreflopRangeCalculator Initial calling range : " << callingRange << endl;
#endif

    // Handle no raises and no calls
    if (nbRaises == 0 && nbCalls == 0 && myPosition != BUTTON && myPosition != SB)
    {
        return -1; // Never limp unless on button or small blind
    }

    // Adjust for limpers
    if (nbRaises == 0 && nbCalls > 0)
    {
        callingRange = adjustCallForLimpers(callingRange);
    }

    // Handle no raises
    if (nbRaises == 0)
    {
        return clampCallingRange(callingRange);
    }

    // Handle raises
    callingRange = adjustCallForRaises(context, callingRange);

    // Tighten range for big bets
    if (isPreflopBigBet)
    {
        callingRange = adjustCallForBigBet(callingRange, potOdd, myCash, context.highestSet, mySet, smallBlind);
    }

    // Adjust for loose/aggressive raiser
    if (shouldAdjustCallForLooseRaiser(context, nbCalls, nbRaises))
    {
        callingRange = std::max(callingRange, 20.0f);
    }

    // Adjust for good odds
    if (shouldCallForGoodOdds(potOdd, myM, myPosition))
    {
        callingRange = 40.0f;
    }

    // Adjust for all-in raiser
    if (shouldCallForAllIn(context, potOdd, nbRaises))
    {
        callingRange = 100.0f;
    }

    return clampCallingRange(callingRange);
}
float PreflopRangeCalculator::adjustCallForLimpers(float callingRange) const
{
#ifdef LOG_POKER_EXEC
    cout << "\t\tPreflopRangeCalculator 1 or more players have limped, but nobody has raised. Adjusting callingRange : "
         << callingRange << " * 1.2 = " << callingRange * 1.2 << endl;
#endif
    return callingRange * 1.2;
}

float PreflopRangeCalculator::clampCallingRange(float callingRange) const
{
    callingRange = std::ceil(callingRange);
    if (callingRange < 1)
        callingRange = 1;
    if (callingRange > 100)
        callingRange = 100;

#ifdef LOG_POKER_EXEC
    cout << "\t\tPreflopRangeCalculator Standard calling range : " << callingRange << "%" << endl;
#endif
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForRaises(const CurrentHandContext& context, float callingRange) const
{
    const int nbRaises = context.preflopRaisesNumber;
    const int nbPlayers = context.nbPlayers;
    const PlayerPosition myPosition = context.myPosition;
    const int nbRunningPlayers = context.nbRunningPlayers;
    std::shared_ptr<Player> lastRaiser = context.preflopLastRaiser;

    PreflopStatistics raiserStats = lastRaiser->getStatistics(nbPlayers).getPreflopStatistics();

    // Use fallback statistics if necessary
    if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        raiserStats = lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics();
    }

    if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getPreflopRaise() != 0)
    {
        callingRange =
            adjustCallForRaiserStats(callingRange, raiserStats, nbRaises, nbPlayers, myPosition, nbRunningPlayers);
    }
    else
    {
        callingRange = adjustCallForNoStats(callingRange, nbRaises);
    }

    return callingRange;
}

float PreflopRangeCalculator::adjustCallForRaiserStats(float callingRange, const PreflopStatistics& raiserStats,
                                                       int nbRaises, int nbPlayers, PlayerPosition myPosition,
                                                       int nbRunningPlayers) const
{
    if ((myPosition == BUTTON || myPosition == CUTOFF) && nbRunningPlayers > 5)
    {
        callingRange = raiserStats.getPreflopRaise() * (nbPlayers > 3 ? 0.7f : 0.9f);
    }
    else
    {
        callingRange = raiserStats.getPreflopRaise() * (nbPlayers > 3 ? 0.5f : 0.7f);
    }

    if (nbRaises == 2)
    {
        callingRange = raiserStats.getPreflop3Bet();
    }
    else if (nbRaises == 3)
    {
        callingRange = raiserStats.getPreflop4Bet();
    }
    else if (nbRaises > 3)
    {
        callingRange = raiserStats.getPreflop4Bet() * 0.5f;
    }

#ifdef LOG_POKER_EXEC
    cout << "\t\tPreflopRangeCalculatoradjusting callingRange to the last raiser's stats, value is now " << callingRange
         << endl;
#endif
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForNoStats(float callingRange, int nbRaises) const
{
    if (nbRaises == 2)
    {
        callingRange /= 2;
    }
    else if (nbRaises == 3)
    {
        callingRange /= 3;
    }
    else if (nbRaises > 3)
    {
        callingRange /= 4;
    }

#ifdef LOG_POKER_EXEC
    cout << "\t\tPreflopRangeCalculator no stats available, callingRange value is now " << callingRange << endl;
#endif
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForBigBet(float callingRange, int potOdd, int myCash, int highestSetOrigin,
                                                  int mySet, int smallBlind) const
{

    const int highestSet = std::min(myCash, highestSetOrigin);

#ifdef LOG_POKER_EXEC
    cout << "PreflopRangeCalculator::adjustCallForBigBet Parameters:" << endl;
    cout << "\tcallingRange: " << callingRange << endl;
    cout << "\tpotOdd: " << potOdd << endl;
    cout << "\tmyCash: " << myCash << endl;
    cout << "\thighestSet: " << highestSet << endl;
    cout << "\tmySet: " << mySet << endl;
    cout << "\tsmallBlind: " << smallBlind << endl;
#endif
    if (potOdd <= 70 && highestSet > smallBlind * 20 && highestSet - mySet > mySet * 6)
    {
        callingRange = 1.5f;
    }
    else if ((potOdd > 70 && potOdd < 85) || (highestSet > smallBlind * 8 && highestSet < smallBlind * 10))
    {
        callingRange *= 0.7f;
    }
    else if ((potOdd >= 85 && potOdd < 95) || (highestSet >= smallBlind * 10 && highestSet < smallBlind * 15))
    {
        callingRange *= 0.5f;
    }
    else if ((potOdd >= 95 && potOdd < 99) || (highestSet >= smallBlind * 15 && highestSet < smallBlind * 20))
    {
        callingRange *= 0.3f;
    }
    else if (potOdd >= 99)
    {
        callingRange *= 0.1f;
    }

#ifdef LOG_POKER_EXEC
    cout << "\t\tPreflopRangeCalculator pot odd is " << potOdd << " : adjusting callingRange, value is now "
         << callingRange << endl;
#endif
    return callingRange;
}

bool PreflopRangeCalculator::shouldAdjustCallForLooseRaiser(const CurrentHandContext& context, int nbCalls,
                                                            int nbRaises) const
{
    std::shared_ptr<Player> lastRaiser = context.preflopLastRaiser;
    const PlayerPosition myPosition = context.myPosition;

    return lastRaiser->isInVeryLooseMode(context.nbPlayers) &&
           (myPosition >= LATE || myPosition == SB || myPosition == BB) && nbCalls == 0 && nbRaises == 1;
}

bool PreflopRangeCalculator::shouldCallForGoodOdds(int potOdd, int myM, PlayerPosition myPosition) const
{
    return potOdd <= 30 && myM > 15 && (myPosition >= LATE || myPosition == SB || myPosition == BB);
}

bool PreflopRangeCalculator::shouldCallForAllIn(const CurrentHandContext& context, int potOdd, int nbRaises) const
{
    std::shared_ptr<Player> lastRaiser = context.preflopLastRaiser;
    const PlayerPosition myPosition = context.myPosition;

    return context.myM > 10 && potOdd <= 20 && nbRaises < 2 && lastRaiser->getAction() == PLAYER_ACTION_ALLIN &&
           (myPosition >= LATE || myPosition == SB || myPosition == BB);
}

float PreflopRangeCalculator::calculatePreflopRaisingRange(CurrentHandContext& context, bool deterministic) const
{
    const int nbRaises = context.preflopRaisesNumber;
    const int nbCalls = context.preflopCallsNumber;
    const int nbPlayers = context.nbPlayers;
    const PlayerPosition myPosition = context.myPosition;
    const bool canBluff = context.myCanBluff;

    float raisingRange = getRange(myPosition, nbPlayers) * 0.8;

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\tInitial raising range : " << raisingRange << endl;
#endif

    if (nbRaises == 0 && nbCalls > 1 && nbPlayers > 3)
    {
        raisingRange = adjustRaiseForLimpers(raisingRange);
    }

    if (nbRaises > 0)
    {
        raisingRange = adjustRaiseForRaiser(context, raisingRange);
    }
    else
    {
        raisingRange = adjustRaiseForNoRaiser(context, raisingRange, canBluff);
    }

    raisingRange = adjustRaiseForStack(context, raisingRange);

    return clampRaiseRange(raisingRange);
}
float PreflopRangeCalculator::adjustRaiseForLimpers(float raisingRange) const
{
#ifdef LOG_POKER_EXEC
    cout << "\t\t2 or more players have limped, but nobody has raised : tightening raising range to "
         << raisingRange * 0.7 << endl;
#endif
    return raisingRange * 0.7;
}

float PreflopRangeCalculator::adjustRaiseForRaiser(const CurrentHandContext& context, float raisingRange) const
{
    const int nbRaises = context.preflopRaisesNumber;
    const int nbPlayers = context.nbPlayers;
    const int potOdd = context.potOdd;
    const int myCash = context.myCash;
    const int mySet = context.mySet;
    const int smallBlind = context.smallBlind;
    const bool isPreflopBigBet = context.isPreflopBigBet;
    std::shared_ptr<Player> lastRaiser = context.preflopLastRaiser;

    PreflopStatistics raiserStats = lastRaiser->getStatistics(nbPlayers).getPreflopStatistics();

    if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
    {
        raiserStats = lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics();
    }

    if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getPreflopRaise() != 0)
    {
        raisingRange = adjustRaiseForRaiserStats(raiserStats, raisingRange, nbRaises, nbPlayers);
    }
    else
    {
        raisingRange = adjustRaiseForNoRaiserStats(raisingRange, nbRaises);
    }

    if (isPreflopBigBet)
    {
        raisingRange = adjustRaiseForBigBet(raisingRange, potOdd, myCash, context.highestSet, mySet, smallBlind);
    }

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForRaiserStats(const PreflopStatistics& raiserStats, float raisingRange,
                                                        int nbRaises, int nbPlayers) const
{
    if (nbRaises == 1)
    {
        raisingRange = std::min(raisingRange, raiserStats.getPreflopRaise() * (nbPlayers > 3 ? 0.5f : 0.7f));
    }
    else if (nbRaises == 2)
    {
        raisingRange = std::min(raisingRange, raiserStats.getPreflop3Bet() * (nbPlayers > 3 ? 0.5f : 0.7f));
    }
    else if (nbRaises == 3)
    {
        raisingRange = std::min(raisingRange, raiserStats.getPreflop4Bet() * (nbPlayers > 3 ? 0.5f : 0.7f));
    }
    else if (nbRaises > 3)
    {
        raisingRange = 0; // Raise with aces only
    }

#ifdef LOG_POKER_EXEC
    cout << "\t\tAdjusting raising range based on raiser stats, value is now " << raisingRange << endl;
#endif

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForNoRaiserStats(float raisingRange, int nbRaises) const
{
    if (nbRaises == 1)
    {
        raisingRange = 2; // 3-bet with top 2%
    }
    else
    {
        raisingRange = 0; // 4-bet with aces only
    }

#ifdef LOG_POKER_EXEC
    cout << "\t\tNo stats available for raiser, adjusting raising range to " << raisingRange << endl;
#endif

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForNoRaiser(const CurrentHandContext& context, float raisingRange,
                                                     bool canBluff, bool deterministic) const
{
    const int nbPlayers = context.nbPlayers;
    const PlayerPosition myPosition = context.myPosition;
    const std::string myCard1 = context.myCard1;
    const std::string myCard2 = context.myCard2;

    if (!isCardsInRange(myCard1, myCard2, RangeManager::getStringRange(nbPlayers, raisingRange)) &&
        (myPosition == SB || myPosition == BUTTON || myPosition == CUTOFF) && canBluff)
    {
        int rand = 0;
        Randomizer::GetRand(1, 3, 1, &rand);
        if (!deterministic && rand == 2)
        {
            raisingRange = 100;

#ifdef LOG_POKER_EXEC
            cout << "\t\tTrying to steal blinds, setting raising range to 100" << endl;
#endif
        }
    }

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForStack(const CurrentHandContext& context, float raisingRange) const
{
    const int myM = context.myM;
    const int nbPlayers = context.nbPlayers;
    const int nbRaises = context.preflopRaisesNumber;

    static const int mToMinimumRange[] = {65, 45, 44, 43, 40, 39, 38, 37, 36, 35, /* M = 1 (full ring) */
                                          34, 33, 33, 33, 33, 33, 33, 32, 31,     /* M = 2 */
                                          30, 30, 29, 28, 28, 28, 28, 28, 27,     /* M = 3 */
                                          27, 27, 27, 27, 26, 26, 26, 26, 25,     /* M = 4 */
                                          25, 25, 25, 25, 25, 25, 24, 23, 23,     /* M = 5 */
                                          23, 23, 23, 23, 22, 22, 22, 21, 20,     /* M = 6 */
                                          20, 20, 20, 20, 20, 19, 19, 18, 17,     /* M = 7 */
                                          17, 17, 17, 17, 17, 16, 16, 16, 15,     /* M = 8 */
                                          15, 15, 15, 15, 15, 14, 14, 14, 13,     /* M = 9 */
                                          13, 12, 11, 10, 9,  8,  7,  6,  5};     /* M = 10 */

    int handsLeft = myM * nbPlayers;
    if (handsLeft < 1)
        handsLeft = 1;

    if (handsLeft < 90)
    {
        float f = mToMinimumRange[handsLeft];
        if (handsLeft > 4 && nbRaises > 0)
        {
            f = f / (nbRaises * 4);
        }
        raisingRange = std::max(f, raisingRange);

#ifdef LOG_POKER_EXEC
        cout << "\t\tHands left: " << handsLeft << ", minimum raising range set to " << f << endl;
#endif
    }

    return raisingRange;
}
float PreflopRangeCalculator::clampRaiseRange(float raisingRange) const
{
    raisingRange = std::ceil(raisingRange);
    if (raisingRange < 0)
        raisingRange = 0;
    if (raisingRange > 100)
        raisingRange = 100;

#ifdef LOG_POKER_EXEC
    cout << "\t\tFinal raising range: " << raisingRange << "%" << endl;
#endif

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForBigBet(float raisingRange, int potOdd, int myCash, int highestSetOrigin,
                                                   int mySet, int smallBlind) const
{
    const int highestSet = std::min(myCash, highestSetOrigin);

#ifdef LOG_POKER_EXEC
    cout << "PreflopRangeCalculator::adjustRaiseForBigBet Parameters:" << endl;
    cout << "\tcallingRange: " << raisingRange << endl;
    cout << "\tpotOdd: " << potOdd << endl;
    cout << "\tmyCash: " << myCash << endl;
    cout << "\thighestSet: " << highestSet << endl;
    cout << "\tmySet: " << mySet << endl;
    cout << "\tsmallBlind: " << smallBlind << endl;
#endif

    if (potOdd <= 70 && highestSet > smallBlind * 20 && highestSet - mySet > mySet * 6)
    {
        raisingRange = 1.5f;
    }
    else if ((potOdd > 70 && potOdd < 85) || (highestSet > smallBlind * 8 && highestSet < smallBlind * 10))
    {
        raisingRange *= 0.6f;
    }
    else if ((potOdd >= 85 && potOdd < 95) || (highestSet >= smallBlind * 10 && highestSet < smallBlind * 15))
    {
        raisingRange *= 0.4f;
    }
    else if ((potOdd >= 95 && potOdd < 99) || (highestSet >= smallBlind * 15 && highestSet < smallBlind * 20))
    {
        raisingRange = std::min(1.0f, raisingRange * 0.3f);
    }
    else if (potOdd >= 99)
    {
        raisingRange = std::min(1.0f, raisingRange * 0.2f);
    }

#ifdef LOG_POKER_EXEC
    cout << "\tAdjusted raising range for big bet: " << raisingRange << endl;
#endif

    return raisingRange;
}
int PreflopRangeCalculator::getRange(PlayerPosition p, const int nbPlayers) const
{

    switch (p)
    {

    case UTG:
        return UTG_STARTING_RANGE[nbPlayers];
        break;
    case UTG_PLUS_ONE:
        return UTG_PLUS_ONE_STARTING_RANGE[nbPlayers];
        break;
    case UTG_PLUS_TWO:
        return UTG_PLUS_TWO_STARTING_RANGE[nbPlayers];
        break;
    case MIDDLE:
        return MIDDLE_STARTING_RANGE[nbPlayers];
        break;
    case MIDDLE_PLUS_ONE:
        return MIDDLE_PLUS_ONE_STARTING_RANGE[nbPlayers];
        break;
    case LATE:
        return LATE_STARTING_RANGE[nbPlayers];
        break;
    case CUTOFF:
        return CUTOFF_STARTING_RANGE[nbPlayers];
        break;
    case BUTTON:
        return BUTTON_STARTING_RANGE[nbPlayers];
        break;
    case SB:
        return SB_STARTING_RANGE[nbPlayers];
        break;
    case BB:
        return BB_STARTING_RANGE[nbPlayers];
        break;
    default:
        return 0;
    }
}

} // namespace pkt::core::player