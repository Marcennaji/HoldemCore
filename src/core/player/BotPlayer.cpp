/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#include "BotPlayer.h"
#include <core/player/strategy/CurrentHandContext.h>
#include <core/player/strategy/IBotStrategy.h>
#include "core/player/strategy/CurrentHandContext.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core::player
{

using namespace std;

BotPlayer::BotPlayer(GameEvents* events, IHandAuditStore* ha, IPlayersStatisticsStore* ps, int id, std::string name,
                     int sC, bool aS, int mB)
    : Player(events, ha, ps, id, name, sC, aS, mB)
{
}

BotPlayer::~BotPlayer()
{
}

void BotPlayer::action()
{

    switch (currentHand->getCurrentRound())
    {

    case GAME_STATE_PREFLOP:
        doPreflopAction();
        break;

    case GAME_STATE_FLOP:
        doFlopAction();
        break;

    case GAME_STATE_TURN:
        doTurnAction();
        break;

    case GAME_STATE_RIVER:
        doRiverAction();
        break;

    default:
        break;
    }

    evaluateBetAmount(); // original bet amount may be modified
    currentHand->getBoard()->collectSets();
    if (myEvents && myEvents->onPotUpdated)
        myEvents->onPotUpdated(currentHand->getBoard()->getPot());

    myTurn = 0;

#ifdef LOG_POKER_EXEC
    cout << endl;
    if (myAction == PLAYER_ACTION_FOLD)
        cout << "FOLD";
    else if (myAction == PLAYER_ACTION_BET)
        cout << "BET " << myBetAmount;
    else if (myAction == PLAYER_ACTION_RAISE)
        cout << "RAISE " << myRaiseAmount;
    else if (myAction == PLAYER_ACTION_CALL)
        cout << "CALL ";
    else if (myAction == PLAYER_ACTION_CHECK)
        cout << "CHECK";
    else if (myAction == PLAYER_ACTION_ALLIN)
        cout << "ALLIN ";
    else if (myAction == PLAYER_ACTION_NONE)
        cout << "NONE";
    else
        cout << "undefined ?";

    cout << endl << "---------------------------------------------------------------------------------" << endl << endl;
#endif

    currentHand->setPreviousPlayerID(myID);

    if (myEvents && myEvents->onActivePlayerActionDone)
        myEvents->onActivePlayerActionDone();

    // currentHand->getGuiInterface()->showCards(myID);//test
}

void BotPlayer::doPreflopAction()
{
    updateCurrentHandContext(GAME_STATE_PREFLOP);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tpreflop raise : "
         << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise() << " % " << endl;
#endif

    myBetAmount = 0;
    bool shouldCall = myStrategy->preflopShouldCall(*myCurrentHandContext);
    myRaiseAmount = myStrategy->preflopShouldRaise(*myCurrentHandContext);

    myPreflopPotOdd = getPotOdd();

    if (myRaiseAmount > 0)
        shouldCall = false;

    // if last to speak, and nobody has raised : I can check
    if (currentHand->getPreflopRaisesNumber() == 0 && !myRaiseAmount && myPosition == BB)
    {
        myAction = PLAYER_ACTION_CHECK;
    }
    else
    {
        if (myBetAmount > 0)
            myAction = PLAYER_ACTION_BET;
        else if (shouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myRaiseAmount > 0)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    if (myAction == PLAYER_ACTION_RAISE || myAction != PLAYER_ACTION_ALLIN)
        currentHand->setPreflopLastRaiserID(myID);

    myCurrentHandActions.m_preflopActions.push_back(myAction);

    updatePreflopStatistics();
    updateCurrentHandContext(GAME_STATE_PREFLOP);

    if (myAction != PLAYER_ACTION_FOLD)
        myRangeEstimator->updateUnplausibleRangesGivenPreflopActions(*myCurrentHandContext);
}
void BotPlayer::doFlopAction()
{
    updateCurrentHandContext(GAME_STATE_FLOP);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tPFR : " << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise()
         << endl;
#endif

    myBetAmount = myStrategy->flopShouldBet(*myCurrentHandContext);
    bool shouldCall = myBetAmount ? false : myStrategy->flopShouldCall(*myCurrentHandContext);
    myRaiseAmount = myBetAmount ? false : myStrategy->flopShouldRaise(*myCurrentHandContext);

    if (myRaiseAmount)
        shouldCall = false;

    if (currentHand->getFlopBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
        myAction = PLAYER_ACTION_CHECK;
    else
    {
        if (myBetAmount)
            myAction = PLAYER_ACTION_BET;
        else if (shouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myRaiseAmount)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    if (myAction == PLAYER_ACTION_BET || myAction == PLAYER_ACTION_RAISE || myAction == PLAYER_ACTION_ALLIN)
        currentHand->setFlopLastRaiserID(myID);

    myCurrentHandActions.m_flopActions.push_back(myAction);

    updateFlopStatistics();
    updateCurrentHandContext(GAME_STATE_FLOP);

    if (myAction != PLAYER_ACTION_FOLD)
        myRangeEstimator->updateUnplausibleRangesGivenFlopActions(*myCurrentHandContext);
}
void BotPlayer::doTurnAction()
{

    updateCurrentHandContext(GAME_STATE_TURN);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tPFR : " << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise()
         << endl;
#endif

    myBetAmount = myStrategy->turnShouldBet(*myCurrentHandContext);
    bool shouldCall = myBetAmount ? false : myStrategy->turnShouldCall(*myCurrentHandContext);
    myRaiseAmount = myBetAmount ? false : myStrategy->turnShouldRaise(*myCurrentHandContext);

    if (myRaiseAmount)
        shouldCall = false;

    if (currentHand->getTurnBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
        myAction = PLAYER_ACTION_CHECK;
    else
    {
        if (myBetAmount)
            myAction = PLAYER_ACTION_BET;
        else if (shouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myRaiseAmount)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    if (myAction == PLAYER_ACTION_BET || myAction == PLAYER_ACTION_RAISE || myAction == PLAYER_ACTION_ALLIN)
        currentHand->setTurnLastRaiserID(myID);

    myCurrentHandActions.m_turnActions.push_back(myAction);

    updateTurnStatistics();
    updateCurrentHandContext(GAME_STATE_TURN);

    if (myAction != PLAYER_ACTION_FOLD)
        myRangeEstimator->updateUnplausibleRangesGivenTurnActions(*myCurrentHandContext);
}
void BotPlayer::doRiverAction()
{

    updateCurrentHandContext(GAME_STATE_RIVER);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tPFR : " << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise()
         << endl;
#endif

    myBetAmount = myStrategy->riverShouldBet(*myCurrentHandContext);
    bool shouldCall = myBetAmount ? false : myStrategy->riverShouldCall(*myCurrentHandContext);
    myRaiseAmount = myBetAmount ? false : myStrategy->riverShouldRaise(*myCurrentHandContext);

    if (myRaiseAmount)
        shouldCall = false;

    if (currentHand->getRiverBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
        myAction = PLAYER_ACTION_CHECK;
    else
    {
        if (myBetAmount)
            myAction = PLAYER_ACTION_BET;
        else if (shouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myRaiseAmount)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    myCurrentHandActions.m_riverActions.push_back(myAction);

    updateRiverStatistics();
    updateCurrentHandContext(GAME_STATE_RIVER);

    if (myAction != PLAYER_ACTION_FOLD)
        myRangeEstimator->updateUnplausibleRangesGivenRiverActions(*myCurrentHandContext);
}
void BotPlayer::evaluateBetAmount()
{

    int highestSet = currentHand->getCurrentBettingRound()->getHighestSet();

    if (myAction == PLAYER_ACTION_CALL)
    {

        // all in
        if (highestSet >= myCash + mySet)
        {
            mySet += myCash;
            myCash = 0;
            myAction = PLAYER_ACTION_ALLIN;
        }
        else
        {
            myCash = myCash - highestSet + mySet;
            mySet = highestSet;
        }
    }

    if (myAction == PLAYER_ACTION_BET)
    {

        // if short stack, just go allin
        if (myBetAmount > (myCash * 0.6))
            myBetAmount = myCash;

        if (myBetAmount >= myCash)
        {
            if (myCash < 2 * currentHand->getSmallBlind())
            {
                // -> full bet rule
                currentHand->getCurrentBettingRound()->setFullBetRule(true);
            }
            currentHand->getCurrentBettingRound()->setMinimumRaise(myCash);
            mySet = myCash;
            myCash = 0;
            myAction = PLAYER_ACTION_ALLIN;
            highestSet = mySet;
        }
        else
        {
            currentHand->getCurrentBettingRound()->setMinimumRaise(myBetAmount);
            myCash = myCash - myBetAmount;
            mySet = myBetAmount;
            highestSet = mySet;
        }
        currentHand->setLastActionPlayerID(myID);
    }

    if (myAction == PLAYER_ACTION_RAISE)
    {

        // short stack, just go allin
        if (myRaiseAmount * 2.5 > myCash && getM() < 10)
            myRaiseAmount = myCash;

        if (currentHand->getCurrentBettingRound()->getFullBetRule())
        { // full bet rule -> only call possible
            // all in
            if (highestSet >= myCash + mySet)
            {
                mySet += myCash;
                myCash = 0;
                myAction = PLAYER_ACTION_ALLIN;
            }
            else
            {
                myCash = myCash - highestSet + mySet;
                mySet = highestSet;
                myAction = PLAYER_ACTION_CALL;
            }
        }
        else
        {
            if (myRaiseAmount < currentHand->getCurrentBettingRound()->getMinimumRaise())
            {
                myRaiseAmount = currentHand->getCurrentBettingRound()->getMinimumRaise();
            }
            // all in
            if (highestSet + myRaiseAmount >= myCash + mySet)
            {
                if (highestSet + currentHand->getCurrentBettingRound()->getMinimumRaise() > myCash + mySet)
                {
                    // perhaps full bet rule
                    if (highestSet >= myCash + mySet)
                    {
                        // only call all-in
                        mySet += myCash;
                        myCash = 0;
                        myAction = PLAYER_ACTION_ALLIN;
                    }
                    else
                    {
                        // raise, but not enough --> full bet rule
                        currentHand->getCurrentBettingRound()->setFullBetRule(true);
                        // lastPlayerAction für Karten umblättern reihenfolge setzrn
                        currentHand->setLastActionPlayerID(myID);

                        mySet += myCash;
                        currentHand->getCurrentBettingRound()->setMinimumRaise(mySet - highestSet);
                        myCash = 0;
                        myAction = PLAYER_ACTION_ALLIN;
                        highestSet = mySet;
                    }
                }
                else
                {
                    currentHand->setLastActionPlayerID(myID);

                    mySet += myCash;
                    currentHand->getCurrentBettingRound()->setMinimumRaise(mySet - highestSet);
                    myCash = 0;
                    myAction = PLAYER_ACTION_ALLIN;
                    highestSet = mySet;
                }
            }
            else
            {
                currentHand->getCurrentBettingRound()->setMinimumRaise(myRaiseAmount);
                myCash = myCash + mySet - highestSet - myRaiseAmount;
                mySet = highestSet + myRaiseAmount;
                highestSet = mySet;
                currentHand->setLastActionPlayerID(myID);
            }
        }
    }

    currentHand->getCurrentBettingRound()->setHighestSet(highestSet);
}

float BotPlayer::calculatePreflopCallingRange(CurrentHandContext& context, bool deterministic) const
{
    return myStrategy->getPreflopRangeCalculator()->calculatePreflopCallingRange(context, deterministic);
}

} // namespace pkt::core::player
