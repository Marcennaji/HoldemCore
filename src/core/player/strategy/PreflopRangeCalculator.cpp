
#include "PreflopRangeCalculator.h"
#include <core/services/ServiceContainer.h>
#include "CurrentHandContext.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"

#include <cmath>

namespace pkt::core::player
{
using namespace std;

PreflopRangeCalculator::PreflopRangeCalculator()
{
    // Initialize with default ranges to prevent crashes
    initializeRanges(45, 8); // Default tight-aggressive ranges for 8-player table
}

PreflopRangeCalculator::PreflopRangeCalculator(std::shared_ptr<pkt::core::ServiceContainer> serviceContainer)
    : myServices(serviceContainer)
{
    // Initialize with default ranges to prevent crashes
    initializeRanges(45, 8); // Default tight-aggressive ranges for 8-player table
}

void PreflopRangeCalculator::ensureServicesInitialized() const
{
    if (!myServices)
    {
        static std::shared_ptr<pkt::core::ServiceContainer> defaultServices =
            std::make_shared<pkt::core::AppServiceContainer>();
        myServices = defaultServices;
    }
}

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

    // we have the UnderTheGun starting ranges. Now, deduce the starting ranges for other positions :

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

float PreflopRangeCalculator::calculatePreflopCallingRange(const CurrentHandContext& ctx) const
{
    ensureServicesInitialized();

    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbCalls = ctx.commonContext.bettingContext.preflopCallsNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition myPosition = ctx.personalContext.position;
    const int potOdd = ctx.commonContext.bettingContext.potOdd;
    const int myCash = ctx.personalContext.cash;
    const bool isPreflopBigBet = ctx.commonContext.bettingContext.isPreflopBigBet;
    const int myTotalBetAmount = ctx.personalContext.totalBetAmount;
    const int smallBlind = ctx.commonContext.smallBlind;
    const int myM = ctx.personalContext.m;

    float callingRange = getRange(myPosition, nbPlayers);

    myServices->logger().verbose("Initial calling range : " + std::to_string(callingRange));

    // Handle no raises and no calls
    if (nbRaises == 0 && nbCalls == 0 && myPosition != Button && myPosition != SmallBlind)
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
    callingRange = adjustCallForRaises(ctx, callingRange);

    // Tighten range for big bets
    if (isPreflopBigBet)
    {
        callingRange =
            adjustCallForBigBet(callingRange, potOdd, myCash, ctx.commonContext.bettingContext.highestBetAmount,
                                myTotalBetAmount, smallBlind);
    }

    // Adjust for loose/aggressive raiser
    if (shouldAdjustCallForLooseRaiser(ctx, nbCalls, nbRaises))
    {
        callingRange = std::max(callingRange, 20.0f);
    }

    // Adjust for good odds
    if (couldCallForGoodOdds(potOdd, myM, myPosition))
    {
        callingRange = 40.0f;
    }

    // Adjust for all-in raiser
    if (couldCallForAllIn(ctx, potOdd, nbRaises))
    {
        callingRange = 100.0f;
    }

    return clampCallingRange(callingRange);
}
float PreflopRangeCalculator::adjustCallForLimpers(float callingRange) const
{
    ensureServicesInitialized();

    myServices->logger().verbose("1 or more players have limped, but nobody has raised. Adjusting callingRange : " +
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

    myServices->logger().verbose("calling range : " + std::to_string(callingRange) + "%");
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForRaises(const CurrentHandContext& ctx, float callingRange) const
{
    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition myPosition = ctx.personalContext.position;
    const int nbActingPlayers = ctx.commonContext.playersContext.actingPlayersList->size();
    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;
    if (!lastRaiser)
    {
        return callingRange;
    }

    PreflopStatistics raiserStats = lastRaiser->getStatisticsUpdater()->getStatistics(nbPlayers).preflopStatistics;

    // Use fallback statistics if necessary
    if (raiserStats.hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        lastRaiser->getStatisticsUpdater()->getStatistics(nbPlayers + 1).preflopStatistics.hands >
            MIN_HANDS_STATISTICS_ACCURATE)
    {
        raiserStats = lastRaiser->getStatisticsUpdater()->getStatistics(nbPlayers + 1).preflopStatistics;
    }

    if (raiserStats.hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getPreflopRaise() != 0)
    {
        callingRange =
            adjustCallForRaiserStats(callingRange, raiserStats, nbRaises, nbPlayers, myPosition, nbActingPlayers);
    }
    else
    {
        callingRange = adjustCallForNoStats(callingRange, nbRaises);
    }

    return callingRange;
}

float PreflopRangeCalculator::adjustCallForRaiserStats(float callingRange, const PreflopStatistics& raiserStats,
                                                       int nbRaises, int nbPlayers, PlayerPosition myPosition,
                                                       int nbActingPlayers) const
{
    ensureServicesInitialized();

    if ((myPosition == Button || myPosition == Cutoff) && nbActingPlayers > 5)
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

    myServices->logger().verbose(
        "PreflopRangeCalculator adjusting callingRange to the last raiser's stats, value is now " +
        std::to_string(callingRange));
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForNoStats(float callingRange, int nbRaises) const
{
    ensureServicesInitialized();

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

    myServices->logger().verbose("No stats available, callingRange value is now " + std::to_string(callingRange));
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForBigBet(float callingRange, int potOdd, int myCash,
                                                  int highestBetAmountOrigin, int myTotalBetAmount,
                                                  int smallBlind) const
{
    ensureServicesInitialized();

    const int highestBetAmount = std::min(myCash, highestBetAmountOrigin);

    if (potOdd <= 70 && highestBetAmount > smallBlind * 20 &&
        highestBetAmount - myTotalBetAmount > myTotalBetAmount * 6)
    {
        callingRange = 1.5f;
    }
    else if ((potOdd > 70 && potOdd < 85) || (highestBetAmount > smallBlind * 8 && highestBetAmount < smallBlind * 10))
    {
        callingRange *= 0.7f;
    }
    else if ((potOdd >= 85 && potOdd < 95) ||
             (highestBetAmount >= smallBlind * 10 && highestBetAmount < smallBlind * 15))
    {
        callingRange *= 0.5f;
    }
    else if ((potOdd >= 95 && potOdd < 99) ||
             (highestBetAmount >= smallBlind * 15 && highestBetAmount < smallBlind * 20))
    {
        callingRange *= 0.3f;
    }
    else if (potOdd >= 99)
    {
        callingRange *= 0.1f;
    }

    myServices->logger().verbose("Pot odd is " + std::to_string(potOdd) + " : adjusting callingRange, value is now " +
                                 std::to_string(callingRange));
    return callingRange;
}

bool PreflopRangeCalculator::shouldAdjustCallForLooseRaiser(const CurrentHandContext& ctx, int nbCalls,
                                                            int nbRaises) const
{
    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;
    const PlayerPosition myPosition = ctx.personalContext.position;

    return lastRaiser && lastRaiser->isInVeryLooseMode(ctx.commonContext.playersContext.nbPlayers) &&
           (myPosition >= Late || myPosition == SmallBlind || myPosition == BigBlind) && nbCalls == 0 && nbRaises == 1;
}

bool PreflopRangeCalculator::couldCallForGoodOdds(int potOdd, int myM, PlayerPosition myPosition) const
{
    return potOdd <= 30 && myM > 15 && (myPosition >= Late || myPosition == SmallBlind || myPosition == BigBlind);
}

bool PreflopRangeCalculator::couldCallForAllIn(const CurrentHandContext& ctx, int potOdd, int nbRaises) const
{
    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;
    const PlayerPosition myPosition = ctx.personalContext.position;

    return ctx.personalContext.m > 10 && potOdd <= 20 && nbRaises < 2 &&
        lastRaiser && lastRaiser->getLastAction().type == ActionType::Allin &&
           (myPosition >= Late || myPosition == SmallBlind || myPosition == BigBlind);
}

float PreflopRangeCalculator::calculatePreflopRaisingRange(const CurrentHandContext& ctx) const
{
    ensureServicesInitialized();

    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbCalls = ctx.commonContext.bettingContext.preflopCallsNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition myPosition = ctx.personalContext.position;

    float raisingRange = getRange(myPosition, nbPlayers) * 0.8;

    myServices->logger().verbose("Initial raising range : " + std::to_string(raisingRange));

    if (nbRaises == 0 && nbCalls > 1 && nbPlayers > 3)
    {
        raisingRange = adjustRaiseForLimpers(raisingRange);
    }

    if (nbRaises > 0)
    {
        raisingRange = adjustRaiseForRaiser(ctx, raisingRange);
    }
    else
    {
        // raisingRange = adjustRaiseForNoRaiser(ctx, raisingRange, canBluff);
        raisingRange = adjustRaiseForNoRaiser(ctx, raisingRange, true);
    }

    raisingRange = adjustRaiseForStack(ctx, raisingRange);

    return clampRaiseRange(raisingRange);
}
float PreflopRangeCalculator::adjustRaiseForLimpers(float raisingRange) const
{
    ensureServicesInitialized();

    myServices->logger().verbose("2 or more players have limped, but nobody has raised : tightening raising range to " +
                                 std::to_string(raisingRange * 0.7));
    return raisingRange * 0.7;
}

float PreflopRangeCalculator::adjustRaiseForRaiser(const CurrentHandContext& ctx, float raisingRange) const
{
    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const int potOdd = ctx.commonContext.bettingContext.potOdd;
    const int myCash = ctx.personalContext.cash;
    const int myTotalBetAmount = ctx.personalContext.totalBetAmount;
    const int smallBlind = ctx.commonContext.smallBlind;
    const bool isPreflopBigBet = ctx.commonContext.bettingContext.isPreflopBigBet;
    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;

    PreflopStatistics raiserStats = lastRaiser->getStatisticsUpdater()->getStatistics(nbPlayers).preflopStatistics;

    if (raiserStats.hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        lastRaiser->getStatisticsUpdater()->getStatistics(nbPlayers + 1).preflopStatistics.hands >
            MIN_HANDS_STATISTICS_ACCURATE)
    {
        raiserStats = lastRaiser->getStatisticsUpdater()->getStatistics(nbPlayers + 1).preflopStatistics;
    }

    if (raiserStats.hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getPreflopRaise() != 0)
    {
        raisingRange = adjustRaiseForRaiserStats(raiserStats, raisingRange, nbRaises, nbPlayers);
    }
    else
    {
        raisingRange = adjustRaiseForNoRaiserStats(raisingRange, nbRaises);
    }

    if (isPreflopBigBet)
    {
        raisingRange =
            adjustRaiseForBigBet(raisingRange, potOdd, myCash, ctx.commonContext.bettingContext.highestBetAmount,
                                 myTotalBetAmount, smallBlind);
    }

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForRaiserStats(const PreflopStatistics& raiserStats, float raisingRange,
                                                        int nbRaises, int nbPlayers) const
{
    ensureServicesInitialized();

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

    myServices->logger().verbose("Adjusting raising range based on raiser stats, value is now " +
                                 std::to_string(raisingRange));

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForNoRaiserStats(float raisingRange, int nbRaises) const
{
    ensureServicesInitialized();

    if (nbRaises == 1)
    {
        raisingRange = 2; // 3-bet with top 2%
    }
    else
    {
        raisingRange = 0; // 4-bet with aces only
    }

    myServices->logger().verbose("No stats available for raiser, adjusting raising range to " +
                                 std::to_string(raisingRange));

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForNoRaiser(const CurrentHandContext& ctx, float raisingRange,
                                                     bool canBluff) const
{
    ensureServicesInitialized();

    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition myPosition = ctx.personalContext.position;

    if (!isCardsInRange(ctx.personalContext.holeCards, RangeEstimator::getStringRange(nbPlayers, raisingRange)) &&
        (myPosition == SmallBlind || myPosition == Button || myPosition == Cutoff) && canBluff)
    {
        int rand = 0;
        myServices->randomizer().getRand(1, 3, 1, &rand);
        if (rand == 2)
        {
            raisingRange = 100;

            myServices->logger().verbose("Trying to steal blinds, setting raising range to 100");
        }
    }

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForStack(const CurrentHandContext& ctx, float raisingRange) const
{
    ensureServicesInitialized();

    const int myM = ctx.personalContext.m;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;

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

        myServices->logger().verbose("Hands left: " + std::to_string(handsLeft) + ", minimum raising range set to " +
                                     std::to_string(f));
    }

    return raisingRange;
}
float PreflopRangeCalculator::clampRaiseRange(float raisingRange) const
{
    ensureServicesInitialized();

    raisingRange = std::ceil(raisingRange);
    if (raisingRange < 0)
    {
        raisingRange = 0;
    }
    if (raisingRange > 100)
    {
        raisingRange = 100;
    }

    myServices->logger().verbose("Final raising range: " + std::to_string(raisingRange) + "%");

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForBigBet(float raisingRange, int potOdd, int myCash,
                                                   int highestBetAmountOrigin, int myTotalBetAmount,
                                                   int smallBlind) const
{
    ensureServicesInitialized();

    const int highestBetAmount = std::min(myCash, highestBetAmountOrigin);

    if (potOdd <= 70 && highestBetAmount > smallBlind * 20 &&
        highestBetAmount - myTotalBetAmount > myTotalBetAmount * 6)
    {
        raisingRange = 1.5f;
    }
    else if ((potOdd > 70 && potOdd < 85) || (highestBetAmount > smallBlind * 8 && highestBetAmount < smallBlind * 10))
    {
        raisingRange *= 0.6f;
    }
    else if ((potOdd >= 85 && potOdd < 95) ||
             (highestBetAmount >= smallBlind * 10 && highestBetAmount < smallBlind * 15))
    {
        raisingRange *= 0.4f;
    }
    else if ((potOdd >= 95 && potOdd < 99) ||
             (highestBetAmount >= smallBlind * 15 && highestBetAmount < smallBlind * 20))
    {
        raisingRange = std::min(1.0f, raisingRange * 0.3f);
    }
    else if (potOdd >= 99)
    {
        raisingRange = std::min(1.0f, raisingRange * 0.2f);
    }

    myServices->logger().verbose("Adjusted raising range for big bet: " + std::to_string(raisingRange));

    return raisingRange;
}
int PreflopRangeCalculator::getRange(PlayerPosition p, const int nbPlayers) const
{

    switch (p)
    {

    case UnderTheGun:
        return myUtgStartingRange[nbPlayers];
        break;
    case UnderTheGunPlusOne:
        return myUtgPlusOneStartingRange[nbPlayers];
        break;
    case UnderTheGunPlusTwo:
        return myUtgPlusTwoStartingRange[nbPlayers];
        break;
    case Middle:
        return myMiddleStartingRange[nbPlayers];
        break;
    case MiddlePlusOne:
        return myMiddlePlusOneStartingRange[nbPlayers];
        break;
    case Late:
        return myLateStartingRange[nbPlayers];
        break;
    case Cutoff:
        return myCutoffStartingRange[nbPlayers];
        break;
    case Button:
        return myButtonStartingRange[nbPlayers];
        break;
    case SmallBlind:
        return mySmallBlindStartingRange[nbPlayers];
        break;
    case BigBlind:
        return myBigBlindStartingRange[nbPlayers];
        break;
    default:
        return 0;
    }
}

} // namespace pkt::core::player