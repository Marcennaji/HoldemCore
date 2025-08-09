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

    case Preflop:
        doPreflopAction();
        break;

    case Flop:
        doFlopAction();
        break;

    case Turn:
        doTurnAction();
        break;

    case River:
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
        if (currentHand->getCurrentRoundState() == Preflop)
        {
            GlobalServices::instance().logger()->info(
                "--> Preflop estimated range : " + myRangeEstimator->getEstimatedRange() + '\n');
        }
        else if (currentHand->getCurrentRoundState() == Flop)
        {
            GlobalServices::instance().logger()->verbose(
                "--> Flop estimated range : " + myRangeEstimator->getEstimatedRange() + '\n');
        }
        else if (currentHand->getCurrentRoundState() == Turn)
        {
            GlobalServices::instance().logger()->verbose(
                "--> Turn estimated range : " + myRangeEstimator->getEstimatedRange() + '\n');
        }
        else if (currentHand->getCurrentRoundState() == River)
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
    updateCurrentHandContext(Preflop);

    // Log the current state
    std::ostringstream logMessage;
    logMessage << "\tdoPreflopAction for " << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash << ", stack when starting the betting round = " << myCashAtHandStart
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << std::endl;
    GlobalServices::instance().logger()->info(logMessage.str());

    PlayerAction action = myStrategy->decideAction(*myCurrentHandContext);

    // Update the player's action and amounts
    myAction = action.type;
    if (myAction == ActionType::Bet)
        myBetAmount = action.amount;
    if (myAction == ActionType::Raise)
        myRaiseAmount = action.amount;

    // Handle specific preflop logic
    if (myAction == ActionType::Raise || myAction == ActionType::Allin)
    {
        currentHand->setPreflopLastRaiserId(myID);
    }

    // Record the action
    myCurrentHandActions.getPreflopActions().push_back(myAction);

    // Update statistics and ranges
    updatePreflopStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(Preflop);
        myRangeEstimator->updateUnplausibleRangesGivenPreflopActions(*myCurrentHandContext);
    }
}
void BotPlayer::doFlopAction()
{
    updateCurrentHandContext(Flop);

    // Log the current state
    std::ostringstream logMessage;
    logMessage << "\tdoFlopAction for " << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << std::endl;
    GlobalServices::instance().logger()->verbose(logMessage.str());

    // Use the strategy to decide the action
    PlayerAction action = myStrategy->decideAction(*myCurrentHandContext);

    // Update the player's action and amounts
    myAction = action.type;
    if (myAction == ActionType::Bet)
        myBetAmount = action.amount;
    if (myAction == ActionType::Raise)
        myRaiseAmount = action.amount;

    // Handle specific flop logic
    if (myAction == ActionType::Bet || myAction == ActionType::Raise || myAction == ActionType::Allin)
    {
        currentHand->setFlopLastRaiserId(myID);
    }

    // Record the action
    myCurrentHandActions.getFlopActions().push_back(myAction);

    // Update statistics and ranges
    updateFlopStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(Flop);
        myRangeEstimator->updateUnplausibleRangesGivenFlopActions(*myCurrentHandContext);
    }
}
void BotPlayer::doTurnAction()
{
    updateCurrentHandContext(Turn);

    // Log the current state
    std::ostringstream logMessage;
    logMessage << "\tdoTurnAction for " << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << std::endl;
    GlobalServices::instance().logger()->verbose(logMessage.str());

    // Use the strategy to decide the action
    PlayerAction action = myStrategy->decideAction(*myCurrentHandContext);

    // Update the player's action and amounts
    myAction = action.type;
    if (myAction == ActionType::Bet)
        myBetAmount = action.amount;
    if (myAction == ActionType::Raise)
        myRaiseAmount = action.amount;

    // Handle specific turn logic
    if (myAction == ActionType::Bet || myAction == ActionType::Raise || myAction == ActionType::Allin)
    {
        currentHand->setTurnLastRaiserId(myID);
    }

    // Record the action
    myCurrentHandActions.getTurnActions().push_back(myAction);

    // Update statistics and ranges
    updateTurnStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(Turn);
        myRangeEstimator->updateUnplausibleRangesGivenTurnActions(*myCurrentHandContext);
    }
}
void BotPlayer::doRiverAction()
{
    updateCurrentHandContext(River);

    // Log the current state
    std::ostringstream logMessage;
    logMessage << "\tdoRiverAction for " << getPositionLabel(myPosition) << "\t" << myName << "\t"
               << getCardsValueString() << "\t"
               << "stack = " << myCash
               << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() << std::endl;
    GlobalServices::instance().logger()->verbose(logMessage.str());

    // Use the strategy to decide the action
    PlayerAction action = myStrategy->decideAction(*myCurrentHandContext);

    myPreflopPotOdd = getPotOdd();

    // Update the player's action and amounts
    myAction = action.type;
    if (myAction == ActionType::Bet)
        myBetAmount = action.amount;
    if (myAction == ActionType::Raise)
        myRaiseAmount = action.amount;

    // Record the action
    myCurrentHandActions.getRiverActions().push_back(myAction);

    // Update statistics and ranges
    updateRiverStatistics();

    if (myAction != ActionType::Fold)
    {
        updateCurrentHandContext(River);
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

float BotPlayer::calculatePreflopCallingRange(const CurrentHandContext& ctx) const
{
    return myStrategy->getPreflopRangeCalculator()->calculatePreflopCallingRange(ctx);
}

} // namespace pkt::core::player
