// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "BotPlayer.h"
#include <core/player/strategy/CurrentHandContext.h>
#include <core/player/strategy/IBotStrategy.h>
#include <core/services/GlobalServices.h>
#include "core/player/strategy/CurrentHandContext.h"

#include <sstream>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core::player
{

using namespace std;

BotPlayer::BotPlayer(const GameEvents& events, int id, std::string name, int sC, bool aS, int mB)
    : Player(events, id, name, sC, aS, mB)
{
}

BotPlayer::~BotPlayer() = default;

void BotPlayer::action()
{

    switch (currentHand->getCurrentRoundState())
    {

    case GameStatePreflop:
        doPreflopAction();
        break;

    case GameStateFlop:
        doFlopAction();
        break;

    case GameStateTurn:
        doTurnAction();
        break;

    case GameStateRiver:
        doRiverAction();
        break;

    default:
        break;
    }

    evaluateBetAmount(); // original bet amount may be modified
    currentHand->getBoard()->collectSets();
    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(currentHand->getBoard()->getPot());
    }

    myTurn = 0;

    std::ostringstream logMessage;

    if (myAction == PlayerActionFold)
    {
        logMessage << myName + " FOLD";
    }
    else if (myAction == PlayerActionBet)
    {
        logMessage << myName + " BET " << myBetAmount << endl;
    }
    else if (myAction == PlayerActionRaise)
    {
        logMessage << myName + " RAISE " << myRaiseAmount;
    }
    else if (myAction == PlayerActionCall)
    {
        logMessage << myName + " CALL ";
    }
    else if (myAction == PlayerActionCheck)
    {
        logMessage << myName + " CHECK";
    }
    else if (myAction == PlayerActionAllin)
    {
        logMessage << myName + " ALLIN ";
    }
    else if (myAction == PlayerActionNone)
    {
        logMessage << myName + " NONE";
    }
    else
    {
        logMessage << "undefined ?";
    }

    GlobalServices::instance().logger()->info(logMessage.str() + (myAction == PlayerActionFold ? "\n" : ""));

    if (myAction != PlayerActionFold)
    {
        if (currentHand->getCurrentRoundState() == GameStatePreflop)
        {
            GlobalServices::instance().logger()->info(
                "--> Preflop estimated range : " + myRangeEstimator->getEstimatedRange() + '\n');
        }
        else if (currentHand->getCurrentRoundState() == GameStateFlop)
        {
            GlobalServices::instance().logger()->verbose(
                "--> Flop estimated range : " + myRangeEstimator->getEstimatedRange() + '\n');
        }
        else if (currentHand->getCurrentRoundState() == GameStateTurn)
        {
            GlobalServices::instance().logger()->verbose(
                "--> Turn estimated range : " + myRangeEstimator->getEstimatedRange() + '\n');
        }
        else if (currentHand->getCurrentRoundState() == GameStateRiver)
        {
            GlobalServices::instance().logger()->verbose(
                "--> River estimated range : " + myRangeEstimator->getEstimatedRange() + '\n');
        }
    }
    currentHand->setPreviousPlayerId(myID);

    if (myEvents.onActivePlayerActionDone)
    {
        myEvents.onActivePlayerActionDone();
    }
}

void BotPlayer::doPreflopAction()
{
    updateCurrentHandContext(GameStatePreflop);

    std::ostringstream logMessage;
    logMessage << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
               << "\tpreflop raise : "
               << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise() << " % ";
    GlobalServices::instance().logger()->info(logMessage.str());

    myBetAmount = 0;
    bool shouldCall = myStrategy->preflopShouldCall(*myCurrentHandContext);
    myRaiseAmount = myStrategy->preflopShouldRaise(*myCurrentHandContext);

    myPreflopPotOdd = getPotOdd();

    if (myRaiseAmount > 0)
    {
        shouldCall = false;
    }

    // if last to speak, and nobody has raised : I can check
    if (currentHand->getPreflopRaisesNumber() == 0 && !myRaiseAmount && myPosition == BB)
    {
        myAction = PlayerActionCheck;
    }
    else
    {
        if (myBetAmount > 0)
        {
            myAction = PlayerActionBet;
        }
        else if (shouldCall)
        {
            myAction = PlayerActionCall;
        }
        else if (myRaiseAmount > 0)
        {
            myAction = PlayerActionRaise;
        }
        else
        {
            myAction = PlayerActionFold;
        }
    }

    if (myAction == PlayerActionRaise || myAction == PlayerActionAllin)
    {
        currentHand->setPreflopLastRaiserId(myID);
    }

    myCurrentHandActions.m_preflopActions.push_back(myAction);

    updatePreflopStatistics();
    updateCurrentHandContext(GameStatePreflop);

    if (myAction != PlayerActionFold)
    {
        myRangeEstimator->updateUnplausibleRangesGivenPreflopActions(*myCurrentHandContext);
    }
}
void BotPlayer::doFlopAction()
{
    updateCurrentHandContext(GameStateFlop);

    std::ostringstream logMessage;
    logMessage << "\n";
    logMessage << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
               << "\tPFR : " << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise()
               << "\n";
    GlobalServices::instance().logger()->verbose(logMessage.str());

    myBetAmount = myStrategy->flopShouldBet(*myCurrentHandContext);
    bool shouldCall = myBetAmount ? false : myStrategy->flopShouldCall(*myCurrentHandContext);
    myRaiseAmount = myBetAmount ? false : myStrategy->flopShouldRaise(*myCurrentHandContext);

    if (myRaiseAmount)
    {
        shouldCall = false;
    }

    if (currentHand->getFlopBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
    {
        myAction = PlayerActionCheck;
    }
    else
    {
        if (myBetAmount)
        {
            myAction = PlayerActionBet;
        }
        else if (shouldCall)
        {
            myAction = PlayerActionCall;
        }
        else if (myRaiseAmount)
        {
            myAction = PlayerActionRaise;
        }
        else
        {
            myAction = PlayerActionFold;
        }
    }

    if (myAction == PlayerActionBet || myAction == PlayerActionRaise || myAction == PlayerActionAllin)
    {
        currentHand->setFlopLastRaiserId(myID);
    }

    myCurrentHandActions.m_flopActions.push_back(myAction);

    updateFlopStatistics();
    updateCurrentHandContext(GameStateFlop);

    if (myAction != PlayerActionFold)
    {
        myRangeEstimator->updateUnplausibleRangesGivenFlopActions(*myCurrentHandContext);
    }
}
void BotPlayer::doTurnAction()
{

    updateCurrentHandContext(GameStateTurn);

    std::ostringstream logMessage;
    logMessage << "\n";
    logMessage << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
               << "\tPFR : " << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise()
               << "\n";
    GlobalServices::instance().logger()->verbose(logMessage.str());

    myBetAmount = myStrategy->turnShouldBet(*myCurrentHandContext);
    bool shouldCall = myBetAmount ? false : myStrategy->turnShouldCall(*myCurrentHandContext);
    myRaiseAmount = myBetAmount ? false : myStrategy->turnShouldRaise(*myCurrentHandContext);

    if (myRaiseAmount)
    {
        shouldCall = false;
    }

    if (currentHand->getTurnBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
    {
        myAction = PlayerActionCheck;
    }
    else
    {
        if (myBetAmount)
        {
            myAction = PlayerActionBet;
        }
        else if (shouldCall)
        {
            myAction = PlayerActionCall;
        }
        else if (myRaiseAmount)
        {
            myAction = PlayerActionRaise;
        }
        else
        {
            myAction = PlayerActionFold;
        }
    }

    if (myAction == PlayerActionBet || myAction == PlayerActionRaise || myAction == PlayerActionAllin)
    {
        currentHand->setTurnLastRaiserId(myID);
    }

    myCurrentHandActions.m_turnActions.push_back(myAction);

    updateTurnStatistics();
    updateCurrentHandContext(GameStateTurn);

    if (myAction != PlayerActionFold)
    {
        myRangeEstimator->updateUnplausibleRangesGivenTurnActions(*myCurrentHandContext);
    }
}
void BotPlayer::doRiverAction()
{

    updateCurrentHandContext(GameStateRiver);

    std::ostringstream logMessage;
    logMessage << "\n";
    logMessage << "\t" << getPositionLabel(myPosition) << "\t" << myName << "\t" << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
               << "\tPFR : " << getStatistics(myCurrentHandContext->nbPlayers).getPreflopStatistics().getPreflopRaise()
               << "\n";
    GlobalServices::instance().logger()->verbose(logMessage.str());

    myBetAmount = myStrategy->riverShouldBet(*myCurrentHandContext);
    bool shouldCall = myBetAmount ? false : myStrategy->riverShouldCall(*myCurrentHandContext);
    myRaiseAmount = myBetAmount ? false : myStrategy->riverShouldRaise(*myCurrentHandContext);

    if (myRaiseAmount)
    {
        shouldCall = false;
    }

    if (currentHand->getRiverBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
    {
        myAction = PlayerActionCheck;
    }
    else
    {
        if (myBetAmount)
        {
            myAction = PlayerActionBet;
        }
        else if (shouldCall)
        {
            myAction = PlayerActionCall;
        }
        else if (myRaiseAmount)
        {
            myAction = PlayerActionRaise;
        }
        else
        {
            myAction = PlayerActionFold;
        }
    }

    myCurrentHandActions.m_riverActions.push_back(myAction);

    updateRiverStatistics();
    updateCurrentHandContext(GameStateRiver);

    if (myAction != PlayerActionFold)
    {
        myRangeEstimator->updateUnplausibleRangesGivenRiverActions(*myCurrentHandContext);
    }
}
void BotPlayer::evaluateBetAmount()
{

    int highestSet = currentHand->getCurrentBettingRound()->getHighestSet();

    if (myAction == PlayerActionCall)
    {

        // all in
        if (highestSet >= myCash + mySet)
        {
            mySet += myCash;
            myCash = 0;
            myAction = PlayerActionAllin;
        }
        else
        {
            myCash = myCash - highestSet + mySet;
            mySet = highestSet;
        }
    }

    if (myAction == PlayerActionBet)
    {

        // if short stack, just go allin
        if (myBetAmount > (myCash * 0.6))
        {
            myBetAmount = myCash;
        }

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
            myAction = PlayerActionAllin;
            highestSet = mySet;
        }
        else
        {
            currentHand->getCurrentBettingRound()->setMinimumRaise(myBetAmount);
            myCash = myCash - myBetAmount;
            mySet = myBetAmount;
            highestSet = mySet;
        }
        currentHand->setLastActionPlayerId(myID);
    }

    if (myAction == PlayerActionRaise)
    {

        // short stack, just go allin
        if (myRaiseAmount * 2.5 > myCash && getM() < 10)
        {
            myRaiseAmount = myCash;
        }

        if (currentHand->getCurrentBettingRound()->getFullBetRule())
        { // full bet rule -> only call possible
            // all in
            if (highestSet >= myCash + mySet)
            {
                mySet += myCash;
                myCash = 0;
                myAction = PlayerActionAllin;
            }
            else
            {
                myCash = myCash - highestSet + mySet;
                mySet = highestSet;
                myAction = PlayerActionCall;
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
                        myAction = PlayerActionAllin;
                    }
                    else
                    {
                        // raise, but not enough --> full bet rule
                        currentHand->getCurrentBettingRound()->setFullBetRule(true);
                        // lastPlayerAction für Karten umblättern reihenfolge setzrn
                        currentHand->setLastActionPlayerId(myID);

                        mySet += myCash;
                        currentHand->getCurrentBettingRound()->setMinimumRaise(mySet - highestSet);
                        myCash = 0;
                        myAction = PlayerActionAllin;
                        highestSet = mySet;
                    }
                }
                else
                {
                    currentHand->setLastActionPlayerId(myID);

                    mySet += myCash;
                    currentHand->getCurrentBettingRound()->setMinimumRaise(mySet - highestSet);
                    myCash = 0;
                    myAction = PlayerActionAllin;
                    highestSet = mySet;
                }
            }
            else
            {
                currentHand->getCurrentBettingRound()->setMinimumRaise(myRaiseAmount);
                myCash = myCash + mySet - highestSet - myRaiseAmount;
                mySet = highestSet + myRaiseAmount;
                highestSet = mySet;
                currentHand->setLastActionPlayerId(myID);
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
