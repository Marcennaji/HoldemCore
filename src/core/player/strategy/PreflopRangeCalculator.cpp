
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
    : m_services(serviceContainer)
{
    // Initialize with default ranges to prevent crashes
    initializeRanges(45, 8); // Default tight-aggressive ranges for 8-player table
}

// ISP-compliant constructor using focused service interfaces
PreflopRangeCalculator::PreflopRangeCalculator(std::shared_ptr<pkt::core::Logger> logger, std::shared_ptr<pkt::core::Randomizer> randomizer)
    : m_logger(logger), m_randomizer(randomizer)
{
    // Initialize with default ranges to prevent crashes
    initializeRanges(45, 8); // Default tight-aggressive ranges for 8-player table
}

void PreflopRangeCalculator::setServices(std::shared_ptr<pkt::core::Logger> logger, std::shared_ptr<pkt::core::Randomizer> randomizer)
{
    m_logger = logger;
    m_randomizer = randomizer;
}

pkt::core::Logger& PreflopRangeCalculator::getLogger() const
{
    if (m_logger) {
        return *m_logger;
    }
    // Fallback to ServiceContainer for backward compatibility
    ensureServicesInitialized();
    return m_services->logger();
}

pkt::core::Randomizer& PreflopRangeCalculator::getRandomizer() const
{
    if (m_randomizer) {
        return *m_randomizer;
    }
    // Fallback to ServiceContainer for backward compatibility
    ensureServicesInitialized();
    return m_services->randomizer();
}

void PreflopRangeCalculator::ensureServicesInitialized() const
{
    if (!m_services)
    {
        static std::shared_ptr<pkt::core::ServiceContainer> defaultServices =
            std::make_shared<pkt::core::AppServiceContainer>();
        m_services = defaultServices;
    }
}

void PreflopRangeCalculator::initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
{

    const float step = (float) (utgHeadsUpRange - utgFullTableRange) / 8;

    // values are % best hands

    m_utgStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    m_utgStartingRange[2] = utgHeadsUpRange;
    m_utgStartingRange[3] = utgHeadsUpRange - step;
    m_utgStartingRange[4] = utgHeadsUpRange - (2 * step);
    m_utgStartingRange[5] = utgHeadsUpRange - (3 * step);
    m_utgStartingRange[6] = utgHeadsUpRange - (4 * step);
    m_utgStartingRange[7] = utgFullTableRange + (3 * step);
    m_utgStartingRange[8] = utgFullTableRange + (2 * step);
    m_utgStartingRange[9] = utgFullTableRange + step;
    m_utgStartingRange[10] = utgFullTableRange;

    assert(m_utgStartingRange[7] < m_utgStartingRange[6]);

    // we have the UnderTheGun starting ranges. Now, deduce the starting ranges for other positions :

    m_utgPlusOneStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_utgPlusOneStartingRange.size(); i++)
    {
        m_utgPlusOneStartingRange[i] = min(50, m_utgStartingRange[i] + 1);
    }

    m_utgPlusTwoStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_utgPlusTwoStartingRange.size(); i++)
    {
        m_utgPlusTwoStartingRange[i] = min(50, m_utgPlusOneStartingRange[i] + 1);
    }

    m_middleStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_middleStartingRange.size(); i++)
    {
        m_middleStartingRange[i] = min(50, m_utgPlusTwoStartingRange[i] + 1);
    }

    m_middlePlusOneStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_middlePlusOneStartingRange.size(); i++)
    {
        m_middlePlusOneStartingRange[i] = min(50, m_middleStartingRange[i] + 1);
    }

    m_lateStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_lateStartingRange.size(); i++)
    {
        m_lateStartingRange[i] = min(50, m_middlePlusOneStartingRange[i] + 1);
    }

    m_cutoffStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_cutoffStartingRange.size(); i++)
    {
        m_cutoffStartingRange[i] = min(50, m_lateStartingRange[i] + 1);
    }

    m_buttonStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_buttonStartingRange.size(); i++)
    {
        m_buttonStartingRange[i] = min(50, m_cutoffStartingRange[i] + 1);
    }

    m_smallBlindStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_smallBlindStartingRange.size(); i++)
    {
        m_smallBlindStartingRange[i] = m_cutoffStartingRange[i];
    }

    m_bigBlindStartingRange.resize(MAX_NUMBER_OF_PLAYERS + 1);
    for (int i = 2; i < m_bigBlindStartingRange.size(); i++)
    {
        m_bigBlindStartingRange[i] = m_smallBlindStartingRange[i] + 1;
    }
}

float PreflopRangeCalculator::calculatePreflopCallingRange(const CurrentHandContext& ctx) const
{
    ensureServicesInitialized();

    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbCalls = ctx.commonContext.bettingContext.preflopCallsNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition m_position = ctx.personalContext.position;
    const int potOdd = ctx.commonContext.bettingContext.potOdd;
    const int m_cash = ctx.personalContext.cash;
    const bool isPreflopBigBet = ctx.commonContext.bettingContext.isPreflopBigBet;
    const int m_totalBetAmount = ctx.personalContext.totalBetAmount;
    const int smallBlind = ctx.commonContext.smallBlind;
    const int m_m = ctx.personalContext.m;

    float callingRange = getRange(m_position, nbPlayers);

    getLogger().verbose("Initial calling range : " + std::to_string(callingRange));

    // Handle no raises and no calls
    if (nbRaises == 0 && nbCalls == 0 && m_position != Button && m_position != SmallBlind)
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
            adjustCallForBigBet(callingRange, potOdd, m_cash, ctx.commonContext.bettingContext.highestBetAmount,
                                m_totalBetAmount, smallBlind);
    }

    // Adjust for loose/aggressive raiser
    if (shouldAdjustCallForLooseRaiser(ctx, nbCalls, nbRaises))
    {
        callingRange = std::max(callingRange, 20.0f);
    }

    // Adjust for good odds
    if (couldCallForGoodOdds(potOdd, m_m, m_position))
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

    getLogger().verbose("1 or more players have limped, but nobody has raised. Adjusting callingRange : " +
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

    getLogger().verbose("calling range : " + std::to_string(callingRange) + "%");
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForRaises(const CurrentHandContext& ctx, float callingRange) const
{
    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition m_position = ctx.personalContext.position;
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
            adjustCallForRaiserStats(callingRange, raiserStats, nbRaises, nbPlayers, m_position, nbActingPlayers);
    }
    else
    {
        callingRange = adjustCallForNoStats(callingRange, nbRaises);
    }

    return callingRange;
}

float PreflopRangeCalculator::adjustCallForRaiserStats(float callingRange, const PreflopStatistics& raiserStats,
                                                       int nbRaises, int nbPlayers, PlayerPosition m_position,
                                                       int nbActingPlayers) const
{
    ensureServicesInitialized();

    if ((m_position == Button || m_position == Cutoff) && nbActingPlayers > 5)
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

    getLogger().verbose(
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

        getLogger().verbose("No stats available, callingRange value is now " + std::to_string(callingRange));
    return callingRange;
}

float PreflopRangeCalculator::adjustCallForBigBet(float callingRange, int potOdd, int m_cash,
                                                  int highestBetAmountOrigin, int m_totalBetAmount,
                                                  int smallBlind) const
{
    ensureServicesInitialized();

    const int highestBetAmount = std::min(m_cash, highestBetAmountOrigin);

    if (potOdd <= 70 && highestBetAmount > smallBlind * 20 &&
        highestBetAmount - m_totalBetAmount > m_totalBetAmount * 6)
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

    getLogger().verbose("Pot odd is " + std::to_string(potOdd) + " : adjusting callingRange, value is now " +
                                 std::to_string(callingRange));
    return callingRange;
}

bool PreflopRangeCalculator::shouldAdjustCallForLooseRaiser(const CurrentHandContext& ctx, int nbCalls,
                                                            int nbRaises) const
{
    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;
    const PlayerPosition m_position = ctx.personalContext.position;

    return lastRaiser && lastRaiser->isInVeryLooseMode(ctx.commonContext.playersContext.nbPlayers) &&
           (m_position >= Late || m_position == SmallBlind || m_position == BigBlind) && nbCalls == 0 && nbRaises == 1;
}

bool PreflopRangeCalculator::couldCallForGoodOdds(int potOdd, int m_m, PlayerPosition m_position) const
{
    return potOdd <= 30 && m_m > 15 && (m_position >= Late || m_position == SmallBlind || m_position == BigBlind);
}

bool PreflopRangeCalculator::couldCallForAllIn(const CurrentHandContext& ctx, int potOdd, int nbRaises) const
{
    std::shared_ptr<Player> lastRaiser = ctx.commonContext.playersContext.preflopLastRaiser;
    const PlayerPosition m_position = ctx.personalContext.position;

    return ctx.personalContext.m > 10 && potOdd <= 20 && nbRaises < 2 &&
        lastRaiser && lastRaiser->getLastAction().type == ActionType::Allin &&
           (m_position >= Late || m_position == SmallBlind || m_position == BigBlind);
}

float PreflopRangeCalculator::calculatePreflopRaisingRange(const CurrentHandContext& ctx) const
{
    ensureServicesInitialized();

    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbCalls = ctx.commonContext.bettingContext.preflopCallsNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition m_position = ctx.personalContext.position;

    float raisingRange = getRange(m_position, nbPlayers) * 0.8;

    getLogger().verbose("Initial raising range : " + std::to_string(raisingRange));

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

    getLogger().verbose("2 or more players have limped, but nobody has raised : tightening raising range to " +
                                 std::to_string(raisingRange * 0.7));
    return raisingRange * 0.7;
}

float PreflopRangeCalculator::adjustRaiseForRaiser(const CurrentHandContext& ctx, float raisingRange) const
{
    const int nbRaises = ctx.commonContext.bettingContext.preflopRaisesNumber;
    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const int potOdd = ctx.commonContext.bettingContext.potOdd;
    const int m_cash = ctx.personalContext.cash;
    const int m_totalBetAmount = ctx.personalContext.totalBetAmount;
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
            adjustRaiseForBigBet(raisingRange, potOdd, m_cash, ctx.commonContext.bettingContext.highestBetAmount,
                                 m_totalBetAmount, smallBlind);
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

    getLogger().verbose("Adjusting raising range based on raiser stats, value is now " +
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

    getLogger().verbose("No stats available for raiser, adjusting raising range to " +
                                 std::to_string(raisingRange));

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForNoRaiser(const CurrentHandContext& ctx, float raisingRange,
                                                     bool canBluff) const
{
    ensureServicesInitialized();

    const int nbPlayers = ctx.commonContext.playersContext.nbPlayers;
    const PlayerPosition m_position = ctx.personalContext.position;

    if (!isCardsInRange(ctx.personalContext.holeCards, RangeEstimator::getStringRange(nbPlayers, raisingRange)) &&
        (m_position == SmallBlind || m_position == Button || m_position == Cutoff) && canBluff)
    {
        int rand = 0;
        getRandomizer().getRand(1, 3, 1, &rand);
        if (rand == 2)
        {
            raisingRange = 100;

            getLogger().verbose("Trying to steal blinds, setting raising range to 100");
        }
    }

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForStack(const CurrentHandContext& ctx, float raisingRange) const
{
    ensureServicesInitialized();

    const int m_m = ctx.personalContext.m;
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

    int handsLeft = m_m * nbPlayers;
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

        getLogger().verbose("Hands left: " + std::to_string(handsLeft) + ", minimum raising range set to " +
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

    getLogger().verbose("Final raising range: " + std::to_string(raisingRange) + "%");

    return raisingRange;
}
float PreflopRangeCalculator::adjustRaiseForBigBet(float raisingRange, int potOdd, int m_cash,
                                                   int highestBetAmountOrigin, int m_totalBetAmount,
                                                   int smallBlind) const
{
    ensureServicesInitialized();

    const int highestBetAmount = std::min(m_cash, highestBetAmountOrigin);

    if (potOdd <= 70 && highestBetAmount > smallBlind * 20 &&
        highestBetAmount - m_totalBetAmount > m_totalBetAmount * 6)
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

    getLogger().verbose("Adjusted raising range for big bet: " + std::to_string(raisingRange));

    return raisingRange;
}
int PreflopRangeCalculator::getRange(PlayerPosition p, const int nbPlayers) const
{

    switch (p)
    {

    case UnderTheGun:
        return m_utgStartingRange[nbPlayers];
        break;
    case UnderTheGunPlusOne:
        return m_utgPlusOneStartingRange[nbPlayers];
        break;
    case UnderTheGunPlusTwo:
        return m_utgPlusTwoStartingRange[nbPlayers];
        break;
    case Middle:
        return m_middleStartingRange[nbPlayers];
        break;
    case MiddlePlusOne:
        return m_middlePlusOneStartingRange[nbPlayers];
        break;
    case Late:
        return m_lateStartingRange[nbPlayers];
        break;
    case Cutoff:
        return m_cutoffStartingRange[nbPlayers];
        break;
    case Button:
        return m_buttonStartingRange[nbPlayers];
        break;
    case SmallBlind:
        return m_smallBlindStartingRange[nbPlayers];
        break;
    case BigBlind:
        return m_bigBlindStartingRange[nbPlayers];
        break;
    default:
        return 0;
    }
}

} // namespace pkt::core::player