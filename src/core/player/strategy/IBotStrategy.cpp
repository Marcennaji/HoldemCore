#include "IBotStrategy.h"
#include <core/engine/Randomizer.h>
#include <core/engine/model/Ranges.h>
#include <core/player/Helpers.h>
#include "CurrentHandContext.h"

using namespace std;

namespace pkt::core::player
{

void IBotStrategy::initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange)
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

float IBotStrategy::getPreflopCallingRange(CurrentHandContext& context, bool deterministic) const
{

    const int nbRaises = context.preflopRaisesNumber;
    const int nbCalls = context.preflopCallsNumber;
    const int nbPlayers = context.nbPlayers;
    const int nbRunningPlayers = context.nbRunningPlayers;
    const PlayerPosition myPosition = context.myPosition;
    std::vector<PlayerPosition> callersPositions = context.callersPositions;
    const int potOdd = context.potOdd;
    const int myCash = context.myCash;
    const bool isPreflopBigBet = context.isPreflopBigBet;
    const int highestSet = context.highestSet;
    const int mySet = context.mySet;
    const int smallBlind = context.smallBlind;
    const int myM = context.myM;

    float callingRange = getRange(myPosition, nbPlayers);

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\tInitial calling range : " << callingRange << endl;
#endif

    if (nbRaises == 0 && nbCalls == 0 && myPosition != BUTTON && myPosition != SB)
        // never limp if nobody has limped, except on button or small blind
        return -1;

    if (nbRaises == 0 && nbCalls > 0)
    { // 1 or more players have limped, but nobody has raised
#ifdef LOG_POKER_EXEC
        cout << "\t\t1 or more players have limped, but nobody has raised. Adjusting callingRange : " << callingRange
             << " * 1.2 = " << callingRange * 1.2 << endl;
#endif
        callingRange = callingRange * 1.2;
    }

    if (nbRaises == 0)
    {
        if (callingRange > 100)
            callingRange = 100;
#ifdef LOG_POKER_EXEC
        cout << "\t\tStandard calling range : " << callingRange << "%" << endl;
#endif
        return callingRange;
    }

    // one or more players raised or re-raised :
    std::shared_ptr<Player> lastRaiser = context.preflopLastRaiser;

    PreflopStatistics raiserStats = lastRaiser->getStatistics(nbPlayers).getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
        lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        raiserStats = lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics();

    if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getPreflopRaise() != 0)
    {

        // adjust range according to the last raiser's stats
        if ((myPosition == BUTTON || myPosition == CUTOFF) && nbRunningPlayers > 5)
            callingRange = raiserStats.getPreflopRaise() * (nbPlayers > 3 ? 0.7 : 0.9);
        else
            callingRange = raiserStats.getPreflopRaise() * (nbPlayers > 3 ? 0.5 : 0.7);

        if (nbRaises == 2) // 3bet
            callingRange = raiserStats.getPreflop3Bet();
        else if (nbRaises == 3) // 4bet
            callingRange = raiserStats.getPreflop4Bet();
        else if (nbRaises > 3) // 5bet or more
            callingRange = raiserStats.getPreflop4Bet() * .5;

#ifdef LOG_POKER_EXEC
        cout << "\t\tadjusting callingRange to the last raiser's stats, value is now " << callingRange << endl;
#endif
    }
    else
    {
        // no stats available for the raiser

        if (nbRaises == 2) // 3bet
            callingRange = callingRange / 2;
        else if (nbRaises == 3) // 4bet
            callingRange = callingRange / 3;
        else if (nbRaises > 3) // 5bet or more
            callingRange = callingRange / 4;

#ifdef LOG_POKER_EXEC
        cout << "\t\tno stats available, callingRange value is now " << callingRange << endl;
#endif
    }

    // if big bet, tighten again
    if (isPreflopBigBet)
    {
        const int highestSet = min(myCash, highestSet);

        if (potOdd <= 70 && highestSet > smallBlind * 20 && highestSet - mySet > mySet * 6)
            callingRange = 1.5;
        else if ((potOdd > 70 && potOdd < 85) || (highestSet > smallBlind * 8 && highestSet < smallBlind * 10))
            callingRange = callingRange * 0.7;
        else if ((potOdd >= 85 && potOdd < 95) || (highestSet >= smallBlind * 10 && highestSet < smallBlind * 15))
            callingRange = callingRange * 0.5;
        else if ((potOdd >= 95 && potOdd < 99) || (highestSet >= smallBlind * 15 && highestSet < smallBlind * 20))
            callingRange = callingRange * 0.3;
        else if (potOdd >= 99)
            callingRange = callingRange * 0.1;

#ifdef LOG_POKER_EXEC
        cout << "\t\tpot odd is " << potOdd << " : adjusting callingRange, value is now " << callingRange << endl;
#endif
    }

    // if the player is being loose or agressive for every last hands, adjust our range, if nobody else has called or
    // raised
    if (lastRaiser->isInVeryLooseMode(nbPlayers) && (myPosition >= LATE || myPosition == SB || myPosition == BB) &&
        nbCalls == 0 && nbRaises == 1)
    {

        if (callingRange < 20)
        {
            callingRange = 20;
#ifdef LOG_POKER_EXEC
            cout << "\t\toveragression detected, setting range to " << callingRange << endl;
#endif
        }
    }

    // call if odds are good
    if (potOdd <= 30 && myM > 15 && (myPosition >= LATE || myPosition == SB || myPosition == BB))
    {
        callingRange = 40;
#ifdef LOG_POKER_EXEC
        cout << "\t\tsmall bet (pot odd is " << potOdd << ") : adjusting callingRange, value is now " << callingRange
             << endl;
#endif
    }

    // call if the raiser is allin, and not a big bet
    if (myM > 10 && potOdd <= 20 && nbRaises < 2 && lastRaiser->getAction() == PLAYER_ACTION_ALLIN &&
        (myPosition >= LATE || myPosition == SB || myPosition == BB))
    {
        callingRange = 100;
#ifdef LOG_POKER_EXEC
        cout << "\t\traiser allin and small bet (pot odd is " << potOdd << ") : adjusting callingRange, value is now "
             << callingRange << endl;
#endif
    }

    callingRange = ceil(callingRange);

    if (callingRange < 1)
        callingRange = 1;

    if (callingRange > 100)
        callingRange = 100;

#ifdef LOG_POKER_EXEC
    cout << "\t\tStandard calling range : " << callingRange << "%" << endl;
#endif

    return callingRange;
}

float IBotStrategy::getPreflopRaisingRange(CurrentHandContext& context, bool deterministic) const
{
    const int nbRaises = context.preflopRaisesNumber;
    const int nbCalls = context.preflopCallsNumber;
    const int nbPlayers = context.nbPlayers;
    const int nbRunningPlayers = context.nbRunningPlayers;
    const PlayerPosition myPosition = context.myPosition;
    std::vector<PlayerPosition> callersPositions = context.callersPositions;
    const int potOdd = context.potOdd;
    const int myCash = context.myCash;
    const bool isPreflopBigBet = context.isPreflopBigBet;
    const int highestSet = context.highestSet;
    const int mySet = context.mySet;
    const int smallBlind = context.smallBlind;
    const int myM = context.myM;
    const int myID = context.myID;
    std::shared_ptr<Player> lastRaiser = context.preflopLastRaiser;
    const std::string myCard1 = context.myCard1;
    const std::string myCard2 = context.myCard2;
    const bool canBluff = context.myCanBluff;

    float raisingRange = getRange(myPosition, nbPlayers) * 0.8;

#ifdef LOG_POKER_EXEC
    cout << endl << "\t\tInitial raising range : " << raisingRange << endl;
#endif

    if (nbRaises == 0 && nbCalls > 1 && nbPlayers > 3)
    { // 2 or more players have limped, but nobody has raised : tighten our usual raising range
#ifdef LOG_POKER_EXEC
        cout << "\t\t2 or more players have limped, but nobody has raised : tighten our usual raising range : "
                "adjusting raisingRange, value is now "
             << raisingRange * 0.7 << endl;
#endif
        raisingRange = raisingRange * 0.7;
    }

    if (nbRaises > 0 && lastRaiser->getID() != myID)
    {

        // determine if we should 3bet or 4bet

        PreflopStatistics raiserStats = lastRaiser->getStatistics(nbPlayers).getPreflopStatistics();
        ;

        // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), as they should be more
        // accurate
        if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
            lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
            raiserStats = lastRaiser->getStatistics(nbPlayers + 1).getPreflopStatistics();

        if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getPreflopRaise() != 0)
        {

            // adjust range according to the last raiser's stats

            if (nbRaises == 1)
            {

                // a bet already occured, determine 3-bet range
                if ((myPosition == BUTTON || myPosition == CUTOFF) && nbRunningPlayers > 4)
                    raisingRange = raiserStats.getPreflopRaise() * (nbPlayers > 3 ? 0.7 : 0.9);
                else
                {
                    raisingRange =
                        std::min(raisingRange, raiserStats.getPreflopRaise() * (nbPlayers > 3 ? 0.5f : 0.7f));
                }
            }
            else if (nbRaises == 2) // a 3-bet already occurred, determine 4-bet range
                raisingRange = min(raisingRange, raiserStats.getPreflop3Bet() * (nbPlayers > 3 ? 0.5f : 0.7f));
            else if (nbRaises == 3) // a 4-bet already occurred, determine 5-bet range
                raisingRange = min(raisingRange, raiserStats.getPreflop4Bet() * (nbPlayers > 3 ? 0.5f : 0.7f));
            else if (nbRaises > 3) // a 5-bet already occurred, determine 6-bet range
                raisingRange = 0;  // raise with aces only

#ifdef LOG_POKER_EXEC
            cout << "\t\t1 or more players have raised : adjusting raisingRange, value is now " << raisingRange << endl;
#endif
        }
        else
        {

            // no stats available for last raiser,
            if (nbRaises == 1)
                raisingRange = 2; // 3bet him with top 2%
            else
                raisingRange = 0; // 4bet him with aces only

#ifdef LOG_POKER_EXEC
            cout << "\t\t1 or more players have raised, but no stats available : adjusting raisingRange, value is now "
                 << raisingRange << endl;
#endif
        }

        if (isPreflopBigBet)
        {

            const int highestSet = min(myCash, highestSet);

            if (potOdd <= 70 && highestSet > smallBlind * 20 && highestSet - mySet > mySet * 6)
                raisingRange = 1.5;
            else if ((potOdd > 70 && potOdd < 85) || (highestSet > smallBlind * 8 && highestSet < smallBlind * 10))
                raisingRange = raisingRange * 0.6;
            else if ((potOdd >= 85 && potOdd < 95) || (highestSet >= smallBlind * 10 && highestSet < smallBlind * 15))
                raisingRange = raisingRange * 0.4;
            else if ((potOdd >= 95 && potOdd < 99) || (highestSet >= smallBlind * 15 && highestSet < smallBlind * 20))
                raisingRange = min(1.0f, raisingRange * 0.3f);
            else if (potOdd >= 99)
                raisingRange = min(1.0f, raisingRange * 0.2f);

#ifdef LOG_POKER_EXEC
            cout << "\t\tbig bet with pot odd " << potOdd << " : adjusting raisingRange, value is now " << raisingRange
                 << endl;
#endif

            // if the raiser is being overagressive for last 8 hands or so, adjust our range (if nobody else has raised
            // or called)
            if (lastRaiser->isInVeryLooseMode(nbPlayers) &&
                (myPosition >= LATE || myPosition == SB || myPosition == BB) && nbCalls == 0 && nbRaises == 1)
            {

                if (raisingRange < 15)
                {
                    raisingRange = 15;
#ifdef LOG_POKER_EXEC
                    cout << "\t\toveragression detected, setting range to " << raisingRange << endl;
#endif
                }
            }
        }
    }
    else
    { // no previous raise

        // steal blinds
        if (!isCardsInRange(myCard1, myCard2, getStringRange(nbPlayers, raisingRange)) &&
            (myPosition == SB || myPosition == BUTTON || myPosition == CUTOFF) && canBluff)
        {

            int rand = 0;
            Randomizer::GetRand(1, 3, 1, &rand);
            if (rand == 2)
            {
                raisingRange = 100;
#ifdef LOG_POKER_EXEC
                cout << "\t\ttrying to steal blind, setting raising range to 100" << endl;
#endif
            }
        }
    }

    // adjust my raising range according to my stack :

    // index is remaining hands to play, if i just fold every hand. Value is the minimum corresponding raising range
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

    // hands remaining before beeing broke, only with the blind cost
    int handsLeft = myM * nbPlayers;

    if (handsLeft < 1)
        handsLeft = 1;

    if (handsLeft < 90)
    {

        float f = mToMinimumRange[handsLeft];

        if (handsLeft > 4 && nbRaises > 0)
            f = f / (nbRaises * 4);

        raisingRange = (f > raisingRange ? f : raisingRange);

#ifdef LOG_POKER_EXEC
        cout << "\t\tHands left : " << handsLeft << ", so minimum raising range is set to " << f << endl;
#endif
    }

    raisingRange = ceil(raisingRange);

    if (raisingRange < 0)
        raisingRange = 0;

    if (raisingRange > 100)
        raisingRange = 100;

#ifdef LOG_POKER_EXEC
    cout << "\t\tStandard raising range : " << raisingRange << "% : ";
#endif

    return raisingRange;
}

int IBotStrategy::getRange(PlayerPosition p, const int nbPlayers) const
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

int IBotStrategy::computePreflopRaiseAmount(CurrentHandContext& ctx, bool deterministic)
{
    int myRaiseAmount = 0;

    const int nbRaises = ctx.preflopRaisesNumber;
    const int nbCalls = ctx.preflopCallsNumber;
    const int nbPlayers = ctx.nbPlayers;

    const int bigBlind = ctx.smallBlind * 2;

    if (nbRaises == 0)
    { // first to raise

        myRaiseAmount = (ctx.myM > 8 ? 2 * bigBlind : 1.5 * bigBlind);

        if (nbPlayers > 4)
        { // adjust for position
            if (ctx.myPosition < MIDDLE)
                myRaiseAmount += bigBlind;
            if (ctx.myPosition == BUTTON)
                myRaiseAmount -= ctx.smallBlind;
        }
        if (ctx.preflopCallsNumber > 0) // increase raise amount if there are limpers
            myRaiseAmount += (ctx.preflopCallsNumber * bigBlind);
    }
    else
    {

        int totalPot = ctx.sets;

        if (nbRaises == 1)
        { // will 3bet
            myRaiseAmount = totalPot * (ctx.myPosition > ctx.preflopLastRaiser->getPosition() ? 1.2 : 1.4);
        }
        if (nbRaises > 1)
        { // will 4bet or more
            myRaiseAmount = totalPot * (ctx.myPosition > ctx.preflopLastRaiser->getPosition() ? 1 : 1.2);
        }
    }

    // if i would be commited in the pot with the computed amount, just go allin preflop
    if (myRaiseAmount > (ctx.myCash * 0.3))
        myRaiseAmount = ctx.myCash;

    return myRaiseAmount;
}

bool IBotStrategy::shouldPotControl(CurrentHandContext& ctx, bool deterministic)
{

    assert(ctx.gameState == GAME_STATE_FLOP || ctx.gameState == GAME_STATE_TURN);

    bool potControl = false;
    const int bigBlind = ctx.smallBlind * 2;

    if (ctx.gameState == GAME_STATE_FLOP &&
        !(ctx.preflopRaisesNumber > 0 && ctx.preflopLastRaiser->getID() == ctx.myID && ctx.flopBetsOrRaisesNumber == 0))
    {

        if (ctx.pot >= bigBlind * 20)
        {

            if (ctx.myPostFlopState.IsPocketPair && !ctx.myPostFlopState.IsOverPair)
                potControl = true;

            if (ctx.myPostFlopState.IsFullHousePossible &&
                !(ctx.myPostFlopState.IsTrips || ctx.myPostFlopState.IsFlush || ctx.myPostFlopState.IsFullHouse ||
                  ctx.myPostFlopState.IsQuads))
                potControl = true;

            if ((ctx.myPostFlopState.IsOverPair || ctx.myPostFlopState.IsTopPair) && ctx.mySet > bigBlind * 20)
                potControl = true;
        }
    }
    else

        if (ctx.gameState == GAME_STATE_TURN)
    {

        if (ctx.pot >= bigBlind * 40)
        {

            if (ctx.myPostFlopState.IsPocketPair && !ctx.myPostFlopState.IsOverPair)
                potControl = true;

            if (ctx.myPostFlopState.IsOverPair)
                potControl = true;

            if (ctx.myPostFlopState.IsFullHousePossible &&
                !(ctx.myPostFlopState.IsTrips || ctx.myPostFlopState.IsFlush || ctx.myPostFlopState.IsFullHouse ||
                  ctx.myPostFlopState.IsQuads))
                potControl = true;

            // 2 pairs
            if (ctx.myPostFlopState.IsTwoPair && !ctx.myPostFlopState.IsFullHousePossible)
                potControl = true;

            if (ctx.myPostFlopState.IsTrips && ctx.mySet > bigBlind * 60)
                potControl = true;
        }
    }

#ifdef LOG_POKER_EXEC
    if (potControl)
        cout << "\t\tShould control pot" << endl;
#endif

    return potControl;
}

} // namespace pkt::core::player