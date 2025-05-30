/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#include "BotPlayer.h"
#include <core/player/strategy/CurrentHandContext.h>
#include <core/player/strategy/IBotStrategy.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core
{

using namespace std;

BotPlayer::BotPlayer(GameEvents* events, IHandAuditStore* ha, IPlayersStatisticsStore* ps, int id, PlayerType type,
                     std::string name, int sC, bool aS, bool sotS, int mB)
    : Player(events, ha, ps, id, type, name, sC, aS, sotS, mB)
{
}

BotPlayer::~BotPlayer()
{
}

void BotPlayer::buildPlayerContext(const GameState state)
{

    // Shared game state
    myCurrentHandContext.gameState = state;

    myCurrentHandContext.nbRunningPlayers = currentHand->getRunningPlayerList()->size();
    myCurrentHandContext.lastVPIPPlayer = getPlayerByUniqueId(currentHand->getLastRaiserID());

    myCurrentHandContext.callersPositions = currentHand->getCallersPositions();
    myCurrentHandContext.pot = currentHand->getBoard()->getPot();
    myCurrentHandContext.potOdd = getPotOdd();
    myCurrentHandContext.sets = currentHand->getBoard()->getSets();
    myCurrentHandContext.highestSet = currentHand->getCurrentBettingRound()->getHighestSet();

    myCurrentHandContext.stringBoard = getStringBoard();

    // Player-specific

    myCurrentHandContext.myCash = myCash;
    myCurrentHandContext.mySet = mySet;
    myCurrentHandContext.myM = static_cast<int>(getM());
    myCurrentHandContext.myCurrentHandActions = myCurrentHandActions;
    myCurrentHandContext.myCanBluff = canBluff(state);
    myCurrentHandContext.myHavePosition = Player::getHavePosition(myPosition, currentHand->getRunningPlayerList());

    if (state == GAME_STATE_PREFLOP)
    {
        myCurrentHandContext.isPreflopBigBet = isPreflopBigBet();
        myCurrentHandContext.myPreflopIsAggressor = isAgressor(GAME_STATE_PREFLOP);
        myCurrentHandContext.preflopLastRaiser = getPlayerByUniqueId(currentHand->getPreflopLastRaiserID());
        myCurrentHandContext.preflopRaisesNumber = currentHand->getPreflopRaisesNumber();
        myCurrentHandContext.preflopCallsNumber = currentHand->getPreflopCallsNumber();
    }
    else
    {
        myCurrentHandContext.myPostFlopState = getPostFlopState();

        if (state == GAME_STATE_FLOP)
        {
            myCurrentHandContext.myFlopIsAggressor = isAgressor(GAME_STATE_FLOP);
            myCurrentHandContext.flopBetsOrRaisesNumber = currentHand->getFlopBetsOrRaisesNumber();
            myCurrentHandContext.flopLastRaiser = getPlayerByUniqueId(currentHand->getFlopLastRaiserID());
        }
        else if (state == GAME_STATE_TURN)
        {
            myCurrentHandContext.myTurnIsAggressor = isAgressor(GAME_STATE_TURN);
            myCurrentHandContext.turnBetsOrRaisesNumber = currentHand->getTurnBetsOrRaisesNumber();
            myCurrentHandContext.turnLastRaiser = getPlayerByUniqueId(currentHand->getTurnLastRaiserID());
        }
        else if (state == GAME_STATE_RIVER)
        {
            myCurrentHandContext.myRiverIsAggressor = isAgressor(GAME_STATE_RIVER);
            myCurrentHandContext.riverBetsOrRaisesNumber = currentHand->getRiverBetsOrRaisesNumber();
        }
    }

    if (currentHand->getCurrentBettingRound()->getBettingRoundID() != myCurrentGameState)
    {
        // do this init once per player and betting round
        myCurrentHandContext.myHandSimulation = getHandSimulation();
        myCurrentHandContext.myID = myID;
        myCurrentHandContext.myCard1 = myCard1;
        myCurrentHandContext.myCard2 = myCard2;
        myCurrentHandContext.myPosition = myPosition;
        myCurrentHandContext.nbPlayers = currentHand->getActivePlayerList()->size();
        myCurrentHandContext.smallBlind = currentHand->getSmallBlind();
    }
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

    myCurrentGameState = currentHand->getCurrentBettingRound()->getBettingRoundID();

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

    buildPlayerContext(GAME_STATE_PREFLOP);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tpreflop raise : "
         << getStatistics(myCurrentHandContext.nbPlayers).getPreflopStatistics().getPreflopRaise() << " % " << endl;
#endif

    myShouldBet = 0;
    myShouldCall = myStrategy->preflopShouldCall(myCurrentHandContext);
    myShouldRaise = myStrategy->preflopShouldRaise(myCurrentHandContext);

    myPreflopPotOdd = getPotOdd();

    if (myShouldRaise)
        myShouldCall = false;

    // if last to speak, and nobody has raised : I can check
    if (currentHand->getPreflopRaisesNumber() == 0 && !myShouldRaise && myPosition == BB)
    {
        myAction = PLAYER_ACTION_CHECK;
    }
    else
    {
        if (myShouldBet)
            myAction = PLAYER_ACTION_BET;
        else if (myShouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myShouldRaise)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    if (myAction == PLAYER_ACTION_RAISE || myAction != PLAYER_ACTION_ALLIN)
        currentHand->setPreflopLastRaiserID(myID);

    myCurrentHandActions.m_preflopActions.push_back(myAction);

    updatePreflopStatistics();

    if (myAction != PLAYER_ACTION_FOLD)
        updateUnplausibleRangesGivenPreflopActions();
}
void BotPlayer::doFlopAction()
{

    buildPlayerContext(GAME_STATE_FLOP);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tPFR : " << getStatistics(myCurrentHandContext.nbPlayers).getPreflopStatistics().getPreflopRaise()
         << endl;
#endif

    myShouldBet = myStrategy->flopShouldBet(myCurrentHandContext);
    myShouldCall = myShouldBet ? false : myStrategy->flopShouldCall(myCurrentHandContext);
    myShouldRaise = myShouldBet ? false : myStrategy->flopShouldRaise(myCurrentHandContext);

    if (myShouldRaise)
        myShouldCall = false;

    if (currentHand->getFlopBetsOrRaisesNumber() == 0 && !myShouldRaise && !myShouldBet)
        myAction = PLAYER_ACTION_CHECK;
    else
    {
        if (myShouldBet)
            myAction = PLAYER_ACTION_BET;
        else if (myShouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myShouldRaise)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    if (myAction == PLAYER_ACTION_BET || myAction == PLAYER_ACTION_RAISE || myAction == PLAYER_ACTION_ALLIN)
        currentHand->setFlopLastRaiserID(myID);

    myCurrentHandActions.m_flopActions.push_back(myAction);

    updateFlopStatistics();

    if (myAction != PLAYER_ACTION_FOLD)
        updateUnplausibleRangesGivenFlopActions();
}
void BotPlayer::doTurnAction()
{

    buildPlayerContext(GAME_STATE_TURN);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tPFR : " << getStatistics(myCurrentHandContext.nbPlayers).getPreflopStatistics().getPreflopRaise()
         << endl;
#endif

    myShouldBet = myStrategy->turnShouldBet(myCurrentHandContext);
    myShouldCall = myShouldBet ? false : myStrategy->turnShouldCall(myCurrentHandContext);
    myShouldRaise = myShouldBet ? false : myStrategy->turnShouldRaise(myCurrentHandContext);

    if (myShouldRaise)
        myShouldCall = false;

    if (currentHand->getTurnBetsOrRaisesNumber() == 0 && !myShouldRaise && !myShouldBet)
        myAction = PLAYER_ACTION_CHECK;
    else
    {
        if (myShouldBet)
            myAction = PLAYER_ACTION_BET;
        else if (myShouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myShouldRaise)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    if (myAction == PLAYER_ACTION_BET || myAction == PLAYER_ACTION_RAISE || myAction == PLAYER_ACTION_ALLIN)
        currentHand->setTurnLastRaiserID(myID);

    myCurrentHandActions.m_turnActions.push_back(myAction);

    updateTurnStatistics();

    if (myAction != PLAYER_ACTION_FOLD)
        updateUnplausibleRangesGivenTurnActions();
}
void BotPlayer::doRiverAction()
{

    buildPlayerContext(GAME_STATE_RIVER);

#ifdef LOG_POKER_EXEC
    cout << endl
         << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
         << "stack = " << myCash << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tPFR : " << getStatistics(myCurrentHandContext.nbPlayers).getPreflopStatistics().getPreflopRaise()
         << endl;
#endif

    myShouldBet = myStrategy->riverShouldBet(myCurrentHandContext);
    myShouldCall = myShouldBet ? false : myStrategy->riverShouldCall(myCurrentHandContext);
    myShouldRaise = myShouldBet ? false : myStrategy->riverShouldRaise(myCurrentHandContext);

    if (myShouldRaise)
        myShouldCall = false;

    if (currentHand->getRiverBetsOrRaisesNumber() == 0 && !myShouldRaise && !myShouldBet)
        myAction = PLAYER_ACTION_CHECK;
    else
    {
        if (myShouldBet)
            myAction = PLAYER_ACTION_BET;
        else if (myShouldCall)
            myAction = PLAYER_ACTION_CALL;
        else if (myShouldRaise)
            myAction = PLAYER_ACTION_RAISE;
        else
            myAction = PLAYER_ACTION_FOLD;
    }

    myCurrentHandActions.m_riverActions.push_back(myAction);

    updateRiverStatistics();

    if (myAction != PLAYER_ACTION_FOLD)
        updateUnplausibleRangesGivenRiverActions();
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

} // namespace pkt::core
