
#include "RangePlausibilityChecker.h"
#include <third_party/psim/psim.hpp>
#include "core/player/RangeManager.h"
#include "core/player/typedefs.h"
/*
namespace pkt::core::player
{

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopCheck(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    // the player is in position, he didn't bet on flop, he is not usually passive, and everybody checked on flop :

    if (bHavePosition &&
        !(flop.getAgressionFactor() < 2 && flop.getAgressionFrequency() < 30 &&
          flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE) &&
        (r.UsesFirst || r.UsesSecond))
    {

        // woudn't slow play a medium hand on a dangerous board
        if (!r.IsFullHousePossible &&
            ((r.IsMiddlePair && !r.IsFullHousePossible && currentHand->getRunningPlayerList()->size() < 4) ||
             r.IsTopPair || r.IsOverPair || (r.IsTwoPair && !r.IsFullHousePossible)) &&
            r.IsFlushDrawPossible && r.IsStraightDrawPossible)
            return true;

        // on a non-paired board, he would'nt slow play a straigth, a set or 2 pairs, if a flush draw is possible
        if (!r.IsFullHousePossible && (r.IsTrips || r.IsStraight || r.IsTwoPair) && r.IsFlushDrawPossible)
            return true;

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (!r.IsFullHousePossible && (r.IsTopPair || r.IsOverPair || r.IsTwoPair || r.IsTrips) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;

        // on a paired board, he wouldn't check if he has a pocket overpair
        if (r.IsFullHousePossible && r.IsOverPair)
            return true;
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopBet(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player made a donk bet on the flop, and is not a maniac player : he should have at least a middle or top pair
    // or a draw
    if (!bHavePosition && !isAgressor(GAME_STATE_PREFLOP))
    {

        if (r.IsOverCards || r.StraightOuts >= 8 || r.FlushOuts >= 8)
            return (currentHand->getRunningPlayerList()->size() > 2 ? true : false);

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair)
            {

                if (r.IsFullHousePossible)
                    return true;

                if (!r.IsMiddlePair && !r.IsTopPair && !r.IsOverPair)
                    return true;
            }
        }
    }

    // on a 3 or more players pot : if the player bets in position, he should have at least a middle pair
    if (bHavePosition && currentHand->getRunningPlayerList()->size() > 2)
    {

        if (r.IsOverCards || r.StraightOuts >= 8 || r.FlushOuts >= 8)
            return true;

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair)
            {

                if (r.IsFullHousePossible)
                    return true;

                if (!r.IsMiddlePair && !r.IsTopPair && !r.IsOverPair)
                    return true;
            }
        }
    }

    // on a 3 or more players pot : if the player is first to act, and bets, he should have at least a top pair
    if (nbChecks == 0 && currentHand->getRunningPlayerList()->size() > 2)
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair)
            {

                if (r.IsFullHousePossible)
                    return true;

                if (!r.IsTopPair && !r.IsOverPair)
                    return true;
            }
        }
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopCall(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    if (currentHand->getFlopBetsOrRaisesNumber() > 0 && myAction == PLAYER_ACTION_CALL &&
        !(stats.getWentToShowDown() > 35 && stats.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush || r.IsOverCards || r.FlushOuts >= 8 || r.StraightOuts >= 8))
        {

            if (r.IsNoPair)
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 1 && r.IsOnePair && !r.IsTopPair && !r.IsOverPair)
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 2 && (r.IsOnePair || r.IsOverCards))
                return true;

            if (currentHand->getRunningPlayerList()->size() > 2 && r.IsOnePair && !r.IsTopPair && !r.IsOverPair)
                return true;
        }
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopRaise(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has check-raised the flop, and is not a maniac player : he should have at least a top pair or a draw
    if (nbChecks == 1)
    {

        if ((r.IsOverCards || r.FlushOuts >= 8 || r.StraightOuts >= 8) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair && !r.IsFullHousePossible && !r.IsTopPair && !r.IsOverPair)
                return true;
        }
    }

    // the player has raised or reraised the flop, and is not a maniac player : he should have at least a top pair
    if (nbRaises > 0)
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsFullHousePossible && !r.IsTopPair & !r.IsOverPair)
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 3 && (r.IsOnePair))
                return true;

            if (currentHand->getFlopBetsOrRaisesNumber() > 4 && (r.IsTwoPair))
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenFlopAllin(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (flop.getAgressionFactor() > 3 && flop.getAgressionFrequency() > 50 &&
        flop.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
          r.IsQuads || r.IsStFlush))
    {

        if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
            return true;

        if (r.IsOnePair && !r.IsFullHousePossible && !r.IsTopPair & !r.IsOverPair)
            return true;

        if (currentHand->getFlopBetsOrRaisesNumber() > 3 && (r.IsOnePair))
            return true;

        if (currentHand->getFlopBetsOrRaisesNumber() > 4 && (r.IsTwoPair))
            return true;
    }
    return false;
}
// purpose : remove some unplausible hands, who would normally be in the estimated preflop range
void RangePlausibilityChecker::updateUnplausibleRangesGivenTurnActions(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    const PlayerStatistics& stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());
    const string originalEstimatedRange = myRangeManager->getEstimatedRange();
    string unplausibleRanges;

#ifdef LOG_POKER_EXEC
    std::cout << endl << "\tPlausible range on turn, before update :\t" << myRangeManager->getEstimatedRange() << endl;
#endif

    // update my unplausible hands (unplausible to my opponents eyes), given what I did on turn

    TurnStatistics turn = stats.getTurnStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (turn.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 3 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        turn = getStatistics(nbPlayers + 1).getTurnStatistics();

    PreflopStatistics preflop = stats.getPreflopStatistics();

    // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
    if (preflop.m_hands < MIN_HANDS_STATISTICS_ACCURATE / 2 && nbPlayers < 10 &&
        getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        preflop = getStatistics(nbPlayers + 1).getPreflopStatistics();

    if (isInVeryLooseMode(nbPlayers))
    {
#ifdef LOG_POKER_EXEC
        std::cout << endl
                  << "\tSeems to be (temporarily ?) on very loose mode : estimated range is\t"
                  << myRangeManager->getEstimatedRange() << endl;
#endif
        return;
    }

    std::string stringBoard;
    int board[5];
    currentHand->getBoard()->getCards(board);

    for (int i = 0; i < 4; i++)
    {
        stringBoard += " ";
        string card = CardsValue::CardStringValue[board[i]];
        stringBoard += card;
    }

    int nbRaises = 0;
    int nbBets = 0;
    int nbChecks = 0;
    int nbCalls = 0;

    for (std::vector<PlayerAction>::const_iterator i = myCurrentHandActions.m_turnActions.begin();
         i != myCurrentHandActions.m_turnActions.end(); i++)
    {

        if (*i == PLAYER_ACTION_RAISE || *i == PLAYER_ACTION_ALLIN)
            nbRaises++;
        else if (*i == PLAYER_ACTION_BET)
            nbBets++;
        else if (*i == PLAYER_ACTION_CHECK)
            nbChecks++;
        else if (*i == PLAYER_ACTION_CALL)
            nbCalls++;
    }

    vector<std::string> ranges = myRangeManager->getRangeAtomicValues(myRangeManager->getEstimatedRange());

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        std::string stringHand = s1 + " " + s2;
        PostFlopState r;
        GetHandState((stringHand + stringBoard).c_str(), &r);

        bool removeHand = false;

        if (myAction == PLAYER_ACTION_CALL)
            removeHand = isUnplausibleHandGivenTurnCall(r, nbRaises, nbBets, nbChecks, nbCalls, turn);
        else if (myAction == PLAYER_ACTION_CHECK)
            removeHand = isUnplausibleHandGivenTurnCheck(r, turn);
        else if (myAction == PLAYER_ACTION_RAISE)
            removeHand = isUnplausibleHandGivenTurnRaise(r, nbRaises, nbBets, nbChecks, nbCalls, turn);
        else if (myAction == PLAYER_ACTION_BET)
            removeHand = isUnplausibleHandGivenTurnBet(r, nbChecks, turn);
        else if (myAction == PLAYER_ACTION_ALLIN)
            removeHand = isUnplausibleHandGivenTurnAllin(r, nbRaises, nbBets, nbChecks, nbCalls, turn);

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

    myRangeManager->setEstimatedRange(
        myRangeManager->substractRange(myRangeManager->getEstimatedRange(), unplausibleRanges, stringBoard));

    if (myRangeManager->getEstimatedRange() == "")
    {
        // keep previous range
#ifdef LOG_POKER_EXEC
        cout << "\tCan't remove all plausible ranges, keeping last one" << endl;
#endif
        myRangeManager->setEstimatedRange(originalEstimatedRange);
        unplausibleRanges = "";
    }

#ifdef LOG_POKER_EXEC
    if (unplausibleRanges != "")
        cout << "\tRemoving unplausible ranges : " << unplausibleRanges << endl;
    logUnplausibleHands(GAME_STATE_TURN);
#endif
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnCheck(CurrentHandContext& ctx)
{

    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    // the player is in position, he isn't usually passive, and everybody checked
    if (bHavePosition && !(turn.getAgressionFactor() < 2 && turn.getAgressionFrequency() < 30 &&
                           turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (r.IsPocketPair && r.IsOverPair)
            return true;

        // woudn't slow play a medium hand on a dangerous board, if there was no action on flop
        if (((r.UsesFirst || r.UsesSecond) && currentHand->getFlopBetsOrRaisesNumber() == 0 && r.IsTopPair ||
             (r.IsTwoPair && !r.IsFullHousePossible) || r.IsTrips) &&
            r.IsFlushDrawPossible)
            return true;

        // wouldn't be passive with a decent hand, on position, if more than 1 opponent
        if (((r.UsesFirst || r.UsesSecond) && ((r.IsTwoPair && !r.IsFullHousePossible) || r.IsTrips)) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnBet(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player made a donk bet on turn, and is not a maniac player : he should have at least a top pair
    if (!bHavePosition && !isAgressor(GAME_STATE_FLOP) && currentHand->getFlopBetsOrRaisesNumber() > 0)
    {

        if ((r.IsOverCards || r.FlushOuts >= 8 || r.StraightOuts >= 8) &&
            currentHand->getRunningPlayerList()->size() > 2)
            return true;

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair)
                return true;

            if (r.IsOnePair && !r.IsTopPair && !r.IsOverPair && !r.IsFullHousePossible)
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnCall(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very loose, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player called a bet on flop and turn, and he is not loose
    if (currentHand->getTurnBetsOrRaisesNumber() > 0 && currentHand->getFlopBetsOrRaisesNumber() > 0 &&
        myAction == PLAYER_ACTION_CALL && !isAgressor(GAME_STATE_FLOP) &&
        !(stats.getWentToShowDown() > 30 && stats.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush || r.FlushOuts >= 8 || r.StraightOuts >= 8))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsTopPair && !r.IsOverPair && !r.IsFullHousePossible)
                return true;

            if (currentHand->getTurnBetsOrRaisesNumber() > 2 && r.IsOnePair)
                return true;
        }
    }
    // the player called a raise on turn, and is not loose : he has at least a top pair or a good draw
    if (currentHand->getTurnBetsOrRaisesNumber() > 1 && myAction == PLAYER_ACTION_CALL &&
        !(stats.getWentToShowDown() > 35 && stats.getRiverStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE))
    {

        if (!(((r.IsTwoPair && !r.IsFullHousePossible) && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush ||
              r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush || r.IsOverCards || r.IsFlushDrawPossible ||
              r.FlushOuts >= 8 || r.StraightOuts >= 8))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsTopPair && !r.IsFullHousePossible && !r.IsOverPair)
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnRaise(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // if nobody has bet the flop, he should at least have a top pair
    if (currentHand->getFlopBetsOrRaisesNumber() == 0)
    {

        if (r.IsTopPair || r.IsOverPair || (r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush ||
            r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }
    // if he was not the agressor on flop, and an other player has bet the flop, then he should have at least a top pair
    if (!isAgressor(GAME_STATE_FLOP) && currentHand->getFlopBetsOrRaisesNumber() > 0)
    {

        if (r.IsTopPair || r.IsOverPair || (r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush ||
            r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }
    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (nbRaises == 2 && !((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse ||
                           r.IsTrips || r.IsQuads || r.IsStFlush))
        return true;

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (nbRaises > 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenTurnAllin(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (turn.getAgressionFactor() > 3 && turn.getAgressionFrequency() > 50 &&
        turn.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has raised twice the turn, and is not a maniac player : he should have at least two pairs
    if (nbRaises == 2 && !((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse ||
                           r.IsTrips || r.IsQuads || r.IsStFlush))
        return true;

    // the player has raised 3 times the turn, and is not a maniac player : he should have better than a set
    if (nbRaises > 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverCheck(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    // todo

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverBet(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has bet the river, was not the agressor on turn and river, and is not a maniac player : he should
    // have at least 2 pairs
    if (currentHand->getFlopBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_FLOP) &&
        currentHand->getTurnBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_TURN))
    {

        if ((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
            r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }

    // the player has bet the river, is out of position on a multi-players pot, in a hand with some action, and is
    // not a maniac player : he should have at least 2 pairs
    if (!bHavePosition && currentHand->getRunningPlayerList()->size() > 2 &&
        ((currentHand->getFlopBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_FLOP)) ||
         (currentHand->getTurnBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_TURN))))
    {

        if ((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
            r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }
    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverCall(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has called the river on a multi-players pot, and is not a loose player : he should have at least a
    // top pair
    if (currentHand->getRunningPlayerList()->size() > 2)
    {

        if (!((r.IsTwoPair && !r.IsFullHousePossible) || r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips ||
              r.IsQuads || r.IsStFlush))
        {

            if (r.IsNoPair || (r.IsOnePair && r.IsFullHousePossible))
                return true;

            if (r.IsOnePair && !r.IsTopPair & !r.IsOverPair)
                return true;
        }
    }

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverRaise(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    // the player has raised the river, and is not a maniac player : he should have at least 2 pairs
    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    if (r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush ||
        (r.IsTwoPair && !r.IsFullHousePossible))
        return false;
    else
        return true;

    // the player has raised the river, is out of position on a multi-players pot, in a hand with some action, and
    // is not a maniac player : he should have at least a set
    if (!bHavePosition && currentHand->getRunningPlayerList()->size() > 2 &&
        ((currentHand->getFlopBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_FLOP)) ||
         (currentHand->getTurnBetsOrRaisesNumber() > 1 && !isAgressor(GAME_STATE_TURN))))
    {

        if (r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush)
            return false;
        else
            return true;
    }

    // the player has raised twice the river, and is not a maniac player : he should have at least trips
    if (nbRaises == 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsTrips || r.IsQuads || r.IsStFlush))
        return true;

    // the player has raised 3 times the river, and is not a maniac player : he should have better than a set
    if (nbRaises > 2 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

bool RangePlausibilityChecker::isUnplausibleHandGivenRiverAllin(CurrentHandContext& ctx)
{

    const int nbPlayers = currentHand->getActivePlayerList()->size();
    PlayerStatistics stats = getStatistics(nbPlayers);
    const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (getPotOdd() < 20)
        return false;

    if (river.getAgressionFactor() > 3 && river.getAgressionFrequency() > 50 &&
        river.m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        return false; // he is usually very agressive, so don't make any guess

    if (isInVeryLooseMode(nbPlayers))
        return false; // he is (temporarily ?) very agressive, so don't make any guess

    if (!r.UsesFirst && !r.UsesSecond)
        return true;

    // the player has raised twice or more the river, and is not a maniac player : he should have at least a
    // straight
    if (nbRaises > 1 && !(r.IsStraight || r.IsFlush || r.IsFullHouse || r.IsQuads || r.IsStFlush))
        return true;

    return false;
}

} // namespace pkt::core::player

*/