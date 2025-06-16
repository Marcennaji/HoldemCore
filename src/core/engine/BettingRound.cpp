// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BettingRound.h"

#include "Exception.h"
#include "model/ButtonState.h"
#include "model/EngineError.h"

#include <core/interfaces/ILogger.h>
#include <core/player/BotPlayer.h>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

BettingRound::BettingRound(GameEvents* events, ILogger* logger, IHand* hi, unsigned dP, int sB, GameState gS)
    : myLogger(logger), IBettingRound(), myHand(hi), myBettingRoundID(gS), dealerPosition(dP), smallBlindPosition(0),
      dealerPositionId(dP), smallBlindPositionId(0), bigBlindPositionId(0), smallBlind(sB), highestSet(0),
      minimumRaise(2 * sB), fullBetRule(false), firstRun(true), firstRunGui(true), firstRound(true),
      firstHeadsUpRound(true), currentPlayersTurnId(0), firstRoundLastPlayersTurnId(0), logBoardCardsDone(false),
      myEvents(events)
{
    currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();
    lastPlayersTurnIt = myHand->getRunningPlayerList()->begin();

    PlayerListConstIterator it_c;

    // determine bigBlindPosition
    for (it_c = myHand->getActivePlayerList()->begin(); it_c != myHand->getActivePlayerList()->end(); ++it_c)
    {
        if ((*it_c)->getButton() == BUTTON_BIG_BLIND)
        {
            bigBlindPositionId = (*it_c)->getID();
            break;
        }
    }
    if (it_c == myHand->getActivePlayerList()->end())
    {
        myLogger->error("ACTIVE_PLAYER_NOT_FOUND");
    }

    // determine smallBlindPosition
    for (it_c = myHand->getActivePlayerList()->begin(); it_c != myHand->getActivePlayerList()->end(); ++it_c)
    {
        if ((*it_c)->getButton() == BUTTON_SMALL_BLIND)
        {
            smallBlindPositionId = (*it_c)->getID();
            break;
        }
    }
    if (it_c == myHand->getActivePlayerList()->end())
    {
        myLogger->error("ACTIVE_PLAYER_NOT_FOUND");
    }
}

BettingRound::~BettingRound()
{
}

int BettingRound::getHighestCardsValue() const
{
    myLogger->error("getHighestCardsValue() in wrong BettingRound");
    return 0;
}

void BettingRound::nextPlayer()
{

    PlayerListConstIterator currentPlayersTurnConstIt = myHand->getRunningPlayerIt(currentPlayersTurnId);
    if (currentPlayersTurnConstIt == myHand->getRunningPlayerList()->end())
    {
        myLogger->error("RUNNING_PLAYER_NOT_FOUND");
    }

    auto botPtr = std::dynamic_pointer_cast<BotPlayer>(*currentPlayersTurnConstIt);
    if (botPtr)
        botPtr->action();
}

void BettingRound::run()
{

    if (firstRunGui)
    {
        firstRunGui = false;
        myHand->setPreviousPlayerID(-1);

        if (myEvents && myEvents->onDealBettingRoundCards)
            myEvents->onDealBettingRoundCards(myBettingRoundID);
    }
    else
    {

        if (firstRun)
        {

            firstRun = false;

            if (!(myHand->getAllInCondition()))
            {

                PlayerListIterator it_1, it_2;

                // running player before smallBlind
                bool formerRunningPlayerFound = false;
                if (myHand->getActivePlayerList()->size() > 2)
                {

                    it_1 = myHand->getActivePlayerIt(smallBlindPositionId);
                    if (it_1 == myHand->getActivePlayerList()->end())
                    {
                        myLogger->error("ACTIVE_PLAYER_NOT_FOUND");
                    }

                    for (size_t i = 0; i < myHand->getActivePlayerList()->size(); i++)
                    {

                        if (it_1 == myHand->getActivePlayerList()->begin())
                            it_1 = myHand->getActivePlayerList()->end();
                        --it_1;

                        it_2 = myHand->getRunningPlayerIt((*it_1)->getID());
                        // running player found
                        if (it_2 != myHand->getRunningPlayerList()->end())
                        {
                            firstRoundLastPlayersTurnId = (*it_2)->getID();
                            formerRunningPlayerFound = true;
                            break;
                        }
                    }
                    if (!formerRunningPlayerFound)
                    {
                        myLogger->error("FORMER_RUNNING_PLAYER_NOT_FOUND");
                    }
                }
                // heads up: bigBlind begins -> dealer/smallBlind is running player before bigBlind
                else
                {
                    firstRoundLastPlayersTurnId = smallBlindPositionId;
                }
                currentPlayersTurnId = firstRoundLastPlayersTurnId;
            }
        }

        // log the turned cards
        if (!logBoardCardsDone)
        {

            int tempBoardCardsArray[5];

            myHand->getBoard()->getCards(tempBoardCardsArray);

            switch (myBettingRoundID)
            {
            case GAME_STATE_FLOP:
                myLogger->info("************************* FLOP " + CardsValue::CardStringValue[tempBoardCardsArray[0]] +
                               " " + CardsValue::CardStringValue[tempBoardCardsArray[1]] + " " +
                               CardsValue::CardStringValue[tempBoardCardsArray[2]] + "  *************************");
                break;
            case GAME_STATE_TURN:
                myLogger->info("************************* TURN " + CardsValue::CardStringValue[tempBoardCardsArray[0]] +
                               " " + CardsValue::CardStringValue[tempBoardCardsArray[1]] + " " +
                               CardsValue::CardStringValue[tempBoardCardsArray[2]] + " " +
                               CardsValue::CardStringValue[tempBoardCardsArray[3]] + "  *************************");

                break;
            case GAME_STATE_RIVER:
                myLogger->info("************************* RIVER " +
                               CardsValue::CardStringValue[tempBoardCardsArray[0]] + " " +
                               CardsValue::CardStringValue[tempBoardCardsArray[1]] + " " +
                               CardsValue::CardStringValue[tempBoardCardsArray[2]] + " " +
                               CardsValue::CardStringValue[tempBoardCardsArray[3]] + " " +
                               CardsValue::CardStringValue[tempBoardCardsArray[4]] + "  *************************");

                break;
            default:
                myLogger->error("wrong myBettingRoundID");
            }
            logBoardCardsDone = true;
        }

        bool allHighestSet = true;

        PlayerListIterator it;
        PlayerListIterator it_c;

        for (it_c = myHand->getRunningPlayerList()->begin(); it_c != myHand->getRunningPlayerList()->end(); ++it_c)
        {
            if (highestSet != (*it_c)->getSet())
                allHighestSet = false;
        }

        if (!firstRound && allHighestSet)
        {

            // go to next betting round
            myHand->setCurrentRound(GameState(myBettingRoundID + 1));

            for (it_c = myHand->getRunningPlayerList()->begin(); it_c != myHand->getRunningPlayerList()->end(); ++it_c)
            {
                (*it_c)->setAction(PLAYER_ACTION_NONE);
            }

            myHand->getBoard()->collectSets();
            myHand->getBoard()->collectPot();

            if (myEvents && myEvents->onPotUpdated)
                myEvents->onPotUpdated(myHand->getBoard()->getPot());

            if (myEvents && myEvents->onRefreshSet)
                myEvents->onRefreshSet();

            if (myEvents && myEvents->onRefreshCash)
                myEvents->onRefreshCash();

            for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
            {
                if (myEvents && myEvents->onRefreshAction)
                    myEvents->onRefreshAction(i, PLAYER_ACTION_NONE);
            }

            myHand->switchRounds();
        }
        else
        {

            // determine next running player
            PlayerListConstIterator currentPlayersTurnIt = myHand->getRunningPlayerIt(currentPlayersTurnId);
            if (currentPlayersTurnIt == myHand->getRunningPlayerList()->end())
            {
                myLogger->error("RUNNING_PLAYER_NOT_FOUND");
            }

            ++currentPlayersTurnIt;
            if (currentPlayersTurnIt == myHand->getRunningPlayerList()->end())
                currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();

            currentPlayersTurnId = (*currentPlayersTurnIt)->getID();

            // highlight active players groupbox and clear action
            if (myEvents && myEvents->onRefreshPlayersActiveInactiveStyles)
                myEvents->onRefreshPlayersActiveInactiveStyles(currentPlayersTurnId, 2);

            if (myEvents && myEvents->onRefreshAction)
                myEvents->onRefreshAction(currentPlayersTurnId, 0);

            currentPlayersTurnIt = myHand->getRunningPlayerIt(currentPlayersTurnId);
            if (currentPlayersTurnIt == myHand->getRunningPlayerList()->end())
            {
                myLogger->error("RUNNING_PLAYER_NOT_FOUND");
            }

            (*currentPlayersTurnIt)->setTurn(true);

            if (currentPlayersTurnId == firstRoundLastPlayersTurnId)
            {
                firstRound = false;
            }

            if (currentPlayersTurnId == 0)
            {
                if (myEvents && myEvents->onDoHumanAction)
                    myEvents->onDoHumanAction();
            }
            else
            {
                if (myEvents && myEvents->onBettingRoundAnimation)
                    myEvents->onBettingRoundAnimation(myBettingRoundID);
            }
        }
    }
}
GameState BettingRound::getBettingRoundID() const
{
    return myBettingRoundID;
}

void BettingRound::setHighestCardsValue(int /*theValue*/)
{
}

void BettingRound::setMinimumRaise(int theValue)
{
    minimumRaise = theValue;
}
int BettingRound::getMinimumRaise() const
{
    return minimumRaise;
}

void BettingRound::setFullBetRule(bool theValue)
{
    fullBetRule = theValue;
}
bool BettingRound::getFullBetRule() const
{
    return fullBetRule;
}

void BettingRound::skipFirstRunGui()
{
    firstRunGui = false;
}

void BettingRound::postRiverRun() {};

IHand* BettingRound::getHand() const
{
    return myHand;
}

int BettingRound::getDealerPosition() const
{
    return dealerPosition;
}
void BettingRound::setDealerPosition(int theValue)
{
    dealerPosition = theValue;
}

void BettingRound::setCurrentPlayersTurnId(unsigned theValue)
{
    currentPlayersTurnId = theValue;
}
unsigned BettingRound::getCurrentPlayersTurnId() const
{
    return currentPlayersTurnId;
}

void BettingRound::setFirstRoundLastPlayersTurnId(unsigned theValue)
{
    firstRoundLastPlayersTurnId = theValue;
}
unsigned BettingRound::getFirstRoundLastPlayersTurnId() const
{
    return firstRoundLastPlayersTurnId;
}

void BettingRound::setCurrentPlayersTurnIt(PlayerListIterator theValue)
{
    currentPlayersTurnIt = theValue;
}
PlayerListIterator BettingRound::getCurrentPlayersTurnIt() const
{
    return currentPlayersTurnIt;
}

void BettingRound::setLastPlayersTurnIt(PlayerListIterator theValue)
{
    lastPlayersTurnIt = theValue;
}
PlayerListIterator BettingRound::getLastPlayersTurnIt() const
{
    return lastPlayersTurnIt;
}

void BettingRound::setHighestSet(int theValue)
{
    highestSet = theValue;
}
int BettingRound::getHighestSet() const
{
    return highestSet;
}

void BettingRound::setFirstRun(bool theValue)
{
    firstRun = theValue;
}
bool BettingRound::getFirstRun() const
{
    return firstRun;
}

void BettingRound::setFirstRound(bool theValue)
{
    firstRound = theValue;
}
bool BettingRound::getFirstRound() const
{
    return firstRound;
}

void BettingRound::setDealerPositionId(unsigned theValue)
{
    dealerPositionId = theValue;
}
unsigned BettingRound::getDealerPositionId() const
{
    return dealerPositionId;
}

void BettingRound::setSmallBlindPositionId(unsigned theValue)
{
    smallBlindPositionId = theValue;
}
unsigned BettingRound::getSmallBlindPositionId() const
{
    return smallBlindPositionId;
}

void BettingRound::setBigBlindPositionId(unsigned theValue)
{
    bigBlindPositionId = theValue;
}
unsigned BettingRound::getBigBlindPositionId() const
{
    return bigBlindPositionId;
}

void BettingRound::setSmallBlindPosition(int theValue)
{
    smallBlindPosition = theValue;
}
int BettingRound::getSmallBlindPosition() const
{
    return smallBlindPosition;
}

void BettingRound::setSmallBlind(int theValue)
{
    smallBlind = theValue;
}
int BettingRound::getSmallBlind() const
{
    return smallBlind;
}
} // namespace pkt::core
