
#include "PreflopRangeCalculator.h"
#include <core/services/GlobalServices.h>
#include "CurrentHandContext.h"
#include "core/engine/Randomizer.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"

#include <cmath>

namespace pkt::core::player
{
using namespace std;

void PreflopRangeCalculator::initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
{

    const float step = (float) (utgHeadsUpRange - utgFullTableRange) / 8;

    // values are % best hands

    myUtgStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    myUtgStartingRange[2] = utgHeadsUpRange;
    myUtgStartingRange[3] = utgHeadsUpRange - step;
    myUtgStartingRange[4] = utgHeadsUpRange - (2 * step);
    myUtgStartingRange[5] = utgHeadsUpRange - (3 * step);
    myUtgStartingRange[6] = utgHeadsUpRange - (4 * step);
    myUtgStartingRange[7] = utgFullTableRange + (3 * step);
    myUtgStartingRange[8] = utgFullTableRange + (2 * step);
    myUtgStartingRange[9] = utgFullTableRange + step;
    myUtgStartingRange[10] = utgFullTableRange;

    assert(myUtgStartingRange[7] < myUtgStartingRange[6]);

    // we have the UTG starting ranges. Now, deduce the starting ranges for other positions :

    myUtgPlusOneStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myUtgPlusOneStartingRange.size(); i++)
    {
        myUtgPlusOneStartingRange[i] = min(50, myUtgStartingRange[i] + 1);
    }

    myUtgPlusTwoStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myUtgPlusTwoStartingRange.size(); i++)
    {
        myUtgPlusTwoStartingRange[i] = min(50, myUtgPlusOneStartingRange[i] + 1);
    }

    myMiddleStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myMiddleStartingRange.size(); i++)
    {
        myMiddleStartingRange[i] = min(50, myUtgPlusTwoStartingRange[i] + 1);
    }

    myMiddlePlusOneStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myMiddlePlusOneStartingRange.size(); i++)
    {
        myMiddlePlusOneStartingRange[i] = min(50, myMiddleStartingRange[i] + 1);
    }

    myLateStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myLateStartingRange.size(); i++)
    {
        myLateStartingRange[i] = min(50, myMiddlePlusOneStartingRange[i] + 1);
    }

    myCutoffStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myCutoffStartingRange.size(); i++)
    {
        myCutoffStartingRange[i] = min(50, myLateStartingRange[i] + 1);
    }

    myButtonStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myButtonStartingRange.size(); i++)
    {
        myButtonStartingRange[i] = min(50, myCutoffStartingRange[i] + 1);
    }

    mySmallBlindStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < mySmallBlindStartingRange.size(); i++)
    {
        mySmallBlindStartingRange[i] = myCutoffStartingRange[i];
    }

    myBigBlindStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < myBigBlindStartingRange.size(); i++)
    {
        myBigBlindStartingRange[i] = mySmallBlindStartingRange[i] + 1;
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

    GlobalServices::instance().logger()->verbose("Initial calling range : " + std::to_string(callingRange));

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
    GlobalServices::instance().logger()->verbose(
        "1 or more players have limped, but nobody has raised. Adjusting callingRange : " +
        std::to_string(callingRange) + " * 1.2 = " + std::to_string(callingRange * 1.2));
    return callingRange * 1.2;
}

float PreflopRangeCalculator::clampCallingRange(float callingRange) const
{
    callingRange = std::ceil(callingRange);
    if (callingRange < 1)
    {
        callingRange = 1;
    }
    if (callingRange > 100)
    {
        callingRange = 100;
    }

    GlobalServices::instance().logger()->verbose("calling range : " + std::to_string(callingRange) + "%");
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForRaises(const CurrentHandContext& context, float callingRange) const
{
    const int nbRaises = context.preflopRaisesNumber;
    const int nbPlayers = context.nbPlayers;
    const PlayerPosition myPosition = context.myPosition;
    const int nbRunningPlayers = context.nbRunningPlayers;
    std::shared_ptr<Player> lastRaiser = context.preflopLastRaiser;
    if (!lastRaiser)
    {
        return callingRange;
    }

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

    GlobalServices::instance().logger()->verbose(
        "PreflopRangeCalculator adjusting callingRange to the last raiser's stats, value is now " +
        std::to_string(callingRange));
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

    GlobalServices::instance().logger()->verbose("No stats available, callingRange value is now " +
                                                 std::to_string(callingRange));
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForBigBet(float callingRange, int potOdd, int myCash, int highestSetOrigin,
                                                  int mySet, int smallBlind) const
{

    const int highestSet = std::min(myCash, highestSetOrigin);

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

    GlobalServices::instance().logger()->verbose("Pot odd is " + std::to_string(potOdd) +
                                                 " : adjusting callingRange, value is now " +
                                                 std::to_string(callingRange));
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

    return context.myM > 10 && potOdd <= 20 && nbRaises < 2 && lastRaiser->getAction() == PlayerActionAllin &&
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

    GlobalServices::instance().logger()->verbose("Initial raising range : " + std::to_string(raisingRange));

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
    GlobalServices::instance().logger()->verbose(
        "2 or more players have limped, but nobody has raised : tightening raising range to " +
        std::to_string(raisingRange * 0.7));
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

    GlobalServices::instance().logger()->verbose("Adjusting raising range based on raiser stats, value is now " +
                                                 std::to_string(raisingRange));

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

    GlobalServices::instance().logger()->verbose("No stats available for raiser, adjusting raising range to " +
                                                 std::to_string(raisingRange));

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForNoRaiser(const CurrentHandContext& context, float raisingRange,
                                                     bool canBluff, bool deterministic) const
{
    const int nbPlayers = context.nbPlayers;
    const PlayerPosition myPosition = context.myPosition;
    const std::string myCard1 = context.myCard1;
    const std::string myCard2 = context.myCard2;

    if (!isCardsInRange(myCard1, myCard2, RangeEstimator::getStringRange(nbPlayers, raisingRange)) &&
        (myPosition == SB || myPosition == BUTTON || myPosition == CUTOFF) && canBluff)
    {
        int rand = 0;
        Randomizer::getRand(1, 3, 1, &rand);
        if (!deterministic && rand == 2)
        {
            raisingRange = 100;

            GlobalServices::instance().logger()->verbose("Trying to steal blinds, setting raising range to 100");
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
    {
        handsLeft = 1;
    }

    if (handsLeft < 90)
    {
        float f = mToMinimumRange[handsLeft];
        if (handsLeft > 4 && nbRaises > 0)
        {
            f = f / (nbRaises * 4);
        }
        raisingRange = std::max(f, raisingRange);

        GlobalServices::instance().logger()->verbose("Hands left: " + std::to_string(handsLeft) +
                                                     ", minimum raising range set to " + std::to_string(f));
    }

    return raisingRange;
}
float PreflopRangeCalculator::clampRaiseRange(float raisingRange) const
{
    raisingRange = std::ceil(raisingRange);
    if (raisingRange < 0)
    {
        raisingRange = 0;
    }
    if (raisingRange > 100)
    {
        raisingRange = 100;
    }

    GlobalServices::instance().logger()->verbose("Final raising range: " + std::to_string(raisingRange) + "%");

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForBigBet(float raisingRange, int potOdd, int myCash, int highestSetOrigin,
                                                   int mySet, int smallBlind) const
{
    const int highestSet = std::min(myCash, highestSetOrigin);

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

    GlobalServices::instance().logger()->verbose("Adjusted raising range for big bet: " + std::to_string(raisingRange));

    return raisingRange;
}
int PreflopRangeCalculator::getRange(PlayerPosition p, const int nbPlayers) const
{

    switch (p)
    {

    case UTG:
        return myUtgStartingRange[nbPlayers];
        break;
    case UtgPlusOne:
        return myUtgPlusOneStartingRange[nbPlayers];
        break;
    case UtgPlusTwo:
        return myUtgPlusTwoStartingRange[nbPlayers];
        break;
    case MIDDLE:
        return myMiddleStartingRange[nbPlayers];
        break;
    case MiddlePlusOne:
        return myMiddlePlusOneStartingRange[nbPlayers];
        break;
    case LATE:
        return myLateStartingRange[nbPlayers];
        break;
    case CUTOFF:
        return myCutoffStartingRange[nbPlayers];
        break;
    case BUTTON:
        return myButtonStartingRange[nbPlayers];
        break;
    case SB:
        return mySmallBlindStartingRange[nbPlayers];
        break;
    case BB:
        return myBigBlindStartingRange[nbPlayers];
        break;
    default:
        return 0;
    }
}

} // namespace pkt::core::player