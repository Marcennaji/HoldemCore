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

    if (myAction == ActionType::Fold)
    {
        logMessage << myName + " FOLD";
    }
    else if (myAction == ActionType::Bet)
    {
        logMessage << myName + " BET " << myBetAmount << endl;
    }
    else if (myAction == ActionType::Raise)
    {
        logMessage << myName + " RAISE " << myRaiseAmount;
    }
    else if (myAction == ActionType::Call)
    {
        logMessage << myName + " CALL ";
    }
    else if (myAction == ActionType::Check)
    {
        logMessage << myName + " CHECK";
    }
    else if (myAction == ActionType::Allin)
    {
        logMessage << myName + " ALLIN ";
    }
    else if (myAction == ActionType::None)
    {
        logMessage << myName + " NONE";
    }
    else
    {
        logMessage << "undefined ?";
    }

    GlobalServices::instance().logger()->info(logMessage.str() + (myAction == ActionType::Fold ? "\n" : ""));

    currentHand->recordPlayerAction(currentHand->getCurrentRoundState(), getId(), myAction);

    if (myAction != ActionType::Fold)
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
    currentHand->resolveHandConditions();
}

void BotPlayer::doPreflopAction()
{
    updateCurrentHandContext(GameStatePreflop);

    std::ostringstream logMessage;
    logMessage << "\tdoPreflopAction for" << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash << ", stack when starting the betting round = " << myCashAtHandStart
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << endl;
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
        myAction = ActionType::Check;
    }
    else
    {
        if (myBetAmount > 0)
        {
            myAction = ActionType::Bet;
        }
        else if (shouldCall)
        {
            myAction = ActionType::Call;
        }
        else if (myRaiseAmount > 0)
        {
            myAction = ActionType::Raise;
        }
        else
        {
            myAction = ActionType::Fold;
        }
    }

    if (myAction == ActionType::Raise || myAction == ActionType::Allin)
    {
        currentHand->setPreflopLastRaiserId(myID);
    }

    myCurrentHandActions.getPreflopActions().push_back(myAction);

    updatePreflopStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(GameStatePreflop);
        myRangeEstimator->updateUnplausibleRangesGivenPreflopActions(*myCurrentHandContext);
    }
}
void BotPlayer::doFlopAction()
{
    updateCurrentHandContext(GameStateFlop);

    std::ostringstream logMessage;
    logMessage << "\n";
    logMessage << "\tdoFlopAction for" << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << endl;
    GlobalServices::instance().logger()->verbose(logMessage.str());

    myBetAmount = 0;
    myRaiseAmount = 0;
    bool shouldCall = false;

    if (currentHand->getFlopBetsOrRaisesNumber() == 0)
    {
        myBetAmount = myStrategy->flopShouldBet(*myCurrentHandContext);
    }
    else
    {
        shouldCall = myStrategy->flopShouldCall(*myCurrentHandContext);
        myRaiseAmount = myStrategy->flopShouldRaise(*myCurrentHandContext);
    }

    if (myRaiseAmount)
    {
        shouldCall = false;
    }

    if (currentHand->getFlopBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
    {
        myAction = ActionType::Check;
    }
    else
    {
        if (myBetAmount)
        {
            myAction = ActionType::Bet;
        }
        else if (shouldCall)
        {
            myAction = ActionType::Call;
        }
        else if (myRaiseAmount)
        {
            myAction = ActionType::Raise;
        }
        else
        {
            myAction = ActionType::Fold;
        }
    }

    if (myAction == ActionType::Bet || myAction == ActionType::Raise || myAction == ActionType::Allin)
    {
        currentHand->setFlopLastRaiserId(myID);
    }

    myCurrentHandActions.getFlopActions().push_back(myAction);

    updateFlopStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(GameStateFlop);
        myRangeEstimator->updateUnplausibleRangesGivenFlopActions(*myCurrentHandContext);
    }
}
void BotPlayer::doTurnAction()
{

    updateCurrentHandContext(GameStateTurn);

    std::ostringstream logMessage;
    logMessage << "\n";
    logMessage << "\tdoTurnAction for " << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << endl;
    GlobalServices::instance().logger()->verbose(logMessage.str());

    myBetAmount = 0;
    myRaiseAmount = 0;
    bool shouldCall = false;

    if (currentHand->getTurnBetsOrRaisesNumber() == 0)
    {
        myBetAmount = myStrategy->turnShouldBet(*myCurrentHandContext);
    }
    else
    {
        shouldCall = myStrategy->turnShouldCall(*myCurrentHandContext);
        myRaiseAmount = myStrategy->turnShouldRaise(*myCurrentHandContext);
    }

    if (myRaiseAmount)
    {
        shouldCall = false;
    }

    if (currentHand->getTurnBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
    {
        myAction = ActionType::Check;
    }
    else
    {
        if (myBetAmount)
        {
            myAction = ActionType::Bet;
        }
        else if (shouldCall)
        {
            myAction = ActionType::Call;
        }
        else if (myRaiseAmount)
        {
            myAction = ActionType::Raise;
        }
        else
        {
            myAction = ActionType::Fold;
        }
    }

    if (myAction == ActionType::Bet || myAction == ActionType::Raise || myAction == ActionType::Allin)
    {
        currentHand->setTurnLastRaiserId(myID);
    }

    myCurrentHandActions.getTurnActions().push_back(myAction);

    updateTurnStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(GameStateTurn);
        myRangeEstimator->updateUnplausibleRangesGivenTurnActions(*myCurrentHandContext);
    }
}
void BotPlayer::doRiverAction()
{

    updateCurrentHandContext(GameStateRiver);

    std::ostringstream logMessage;
    logMessage << "\n";
    logMessage << "\tdoRiverAction for " << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << endl;
    GlobalServices::instance().logger()->verbose(logMessage.str());

    myBetAmount = 0;
    myRaiseAmount = 0;
    bool shouldCall = false;

    if (currentHand->getRiverBetsOrRaisesNumber() == 0)
    {
        myBetAmount = myStrategy->riverShouldBet(*myCurrentHandContext);
    }
    else
    {
        shouldCall = myStrategy->riverShouldCall(*myCurrentHandContext);
        myRaiseAmount = myStrategy->riverShouldRaise(*myCurrentHandContext);
    }

    if (myRaiseAmount)
    {
        shouldCall = false;
    }

    if (currentHand->getRiverBetsOrRaisesNumber() == 0 && !myRaiseAmount && !myBetAmount)
    {
        myAction = ActionType::Check;
    }
    else
    {
        if (myBetAmount)
        {
            myAction = ActionType::Bet;
        }
        else if (shouldCall)
        {
            myAction = ActionType::Call;
        }
        else if (myRaiseAmount)
        {
            myAction = ActionType::Raise;
        }
        else
        {
            myAction = ActionType::Fold;
        }
    }

    myCurrentHandActions.getRiverActions().push_back(myAction);

    updateRiverStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(GameStateRiver);
        myRangeEstimator->updateUnplausibleRangesGivenRiverActions(*myCurrentHandContext);
    }
}
void BotPlayer::evaluateBetAmount()
{

    int highestSet = currentHand->getCurrentBettingRound()->getHighestSet();

    if (myAction == ActionType::Call)
    {

        // all in
        if (highestSet >= myCash + mySet)
        {
            mySet += myCash;
            myCash = 0;
            myAction = ActionType::Allin;
        }
        else
        {
            myCash = myCash - highestSet + mySet;
            mySet = highestSet;
        }
    }

    if (myAction == ActionType::Bet)
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
            myAction = ActionType::Allin;
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

    if (myAction == ActionType::Raise)
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
                myAction = ActionType::Allin;
            }
            else
            {
                myCash = myCash - highestSet + mySet;
                mySet = highestSet;
                myAction = ActionType::Call;
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
                        myAction = ActionType::Allin;
                    }
                    else
                    {
                        // raise, but not enough --> full bet rule
                        currentHand->getCurrentBettingRound()->setFullBetRule(true);
                        currentHand->setLastActionPlayerId(myID);

                        mySet += myCash;
                        currentHand->getCurrentBettingRound()->setMinimumRaise(mySet - highestSet);
                        myCash = 0;
                        myAction = ActionType::Allin;
                        highestSet = mySet;
                    }
                }
                else
                {
                    currentHand->setLastActionPlayerId(myID);

                    mySet += myCash;
                    currentHand->getCurrentBettingRound()->setMinimumRaise(mySet - highestSet);
                    myCash = 0;
                    myAction = ActionType::Allin;
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

float BotPlayer::calculatePreflopCallingRange(CurrentHandContext& ctx) const
{
    return myStrategy->getPreflopRangeCalculator()->calculatePreflopCallingRange(ctx);
}

} // namespace pkt::core::player
