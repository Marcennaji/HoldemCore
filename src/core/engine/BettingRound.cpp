// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BettingRound.h"

#include "Exception.h"
#include "model/ButtonState.h"
#include "model/EngineError.h"

#include <core/engine/CardUtilities.h>
#include <core/player/BotPlayer.h>
#include <core/services/GlobalServices.h>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

BettingRound::BettingRound(const GameEvents& events, IHand* hi, unsigned dP, int sB, GameState gS)
    : IBettingRound(), myHand(hi), myBettingRoundID(gS), myDealerPosition(dP), myDealerPositionId(dP), mySmallBlind(sB),
      myMinimumRaise(2 * sB), myEvents(events)
{
    myCurrentPlayersTurnIt = myHand->getRunningPlayersList()->begin();
    myLastPlayersTurnIt = myHand->getRunningPlayersList()->begin();

    PlayerListConstIterator itC;

    // determine bigBlindPosition
    for (itC = myHand->getSeatsList()->begin(); itC != myHand->getSeatsList()->end(); ++itC)
    {
        if ((*itC)->getButton() == ButtonBigBlind)
        {
            myBigBlindPositionId = (*itC)->getId();
            break;
        }
    }
    if (itC == myHand->getSeatsList()->end())
    {
        GlobalServices::instance().logger()->error("ACTIVE_PLAYER_NOT_FOUND");
    }

    // determine smallBlindPosition
    for (itC = myHand->getSeatsList()->begin(); itC != myHand->getSeatsList()->end(); ++itC)
    {
        if ((*itC)->getButton() == ButtonSmallBlind)
        {
            mySmallBlindPositionId = (*itC)->getId();
            break;
        }
    }
    if (itC == myHand->getSeatsList()->end())
    {
        GlobalServices::instance().logger()->error("ACTIVE_PLAYER_NOT_FOUND");
    }
}

BettingRound::~BettingRound() = default;

int BettingRound::getHighestCardsValue() const
{
    GlobalServices::instance().logger()->error("getHighestCardsValue() in wrong BettingRound");
    return 0;
}

void BettingRound::nextPlayer()
{

    PlayerListConstIterator currentPlayersTurnConstIt = myHand->getRunningPlayerIt(myCurrentPlayersTurnId);
    if (currentPlayersTurnConstIt == myHand->getRunningPlayersList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
    }

    auto botPtr = std::dynamic_pointer_cast<BotPlayer>(*currentPlayersTurnConstIt);
    if (botPtr)
    {
        botPtr->action();
    }
}
void BettingRound::run()
{
    GlobalServices::instance().logger()->verbose("myHighestSet at start of round: " + std::to_string(myHighestSet));
    for (auto& player : *myHand->getRunningPlayersList())
    {
        GlobalServices::instance().logger()->verbose("start of round: Player " + player->getName() +
                                                     " action: " + playerActionToString(player->getAction()));
    }
    if (myFirstRunGui)
    {
        handleFirstRunGui();
        return;
    }
    if (myFirstRun)
    {
        handleFirstRun();
    }

    logBoardCards();

    bool allHighestSet = allBetsAreDone();
    GlobalServices::instance().logger()->verbose("allBetsAreDone: " + std::to_string(allHighestSet) +
                                                 ", myFirstRound: " + std::to_string(myFirstRound));

    if (!myFirstRound && allHighestSet)
    {
        GlobalServices::instance().logger()->verbose("all bets are done, proceeding to next betting round");
        proceedToNextBettingRound();
    }
    else
    {
        // determine next running player

        PlayerListConstIterator currentPlayersTurnIt = myHand->getRunningPlayerIt(myCurrentPlayersTurnId);
        if (currentPlayersTurnIt == myHand->getRunningPlayersList()->end())
        {
            currentPlayersTurnIt = myHand->getRunningPlayersList()->begin();
        }

        ++currentPlayersTurnIt;
        if (currentPlayersTurnIt == myHand->getRunningPlayersList()->end())
        {
            currentPlayersTurnIt = myHand->getRunningPlayersList()->begin();
        }

        myCurrentPlayersTurnId = (*currentPlayersTurnIt)->getId();

        // highlight active players groupbox and clear action
        if (myEvents.onRefreshPlayersActiveInactiveStyles)
        {
            myEvents.onRefreshPlayersActiveInactiveStyles(myCurrentPlayersTurnId, 2);
        }

        if (myEvents.onRefreshAction)
        {
            myEvents.onRefreshAction(myCurrentPlayersTurnId, 0);
        }
        GlobalServices::instance().logger()->verbose("BettingRound::run() : Determining next running player (2)");

        currentPlayersTurnIt = myHand->getRunningPlayerIt(myCurrentPlayersTurnId);
        if (currentPlayersTurnIt == myHand->getRunningPlayersList()->end())
        {
            throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
        }

        (*currentPlayersTurnIt)->setTurn(true);

        if (myCurrentPlayersTurnId == myFirstRoundLastPlayersTurnId)
        {
            myFirstRound = false;
        }

        if ((*currentPlayersTurnIt)->getName() == HumanPlayer::getName())
        {
            if (myEvents.onDoHumanAction)
            {
                myEvents.onDoHumanAction();
            }
        }
        else
        {
            if (myEvents.onBettingRoundAnimation)
            {
                myEvents.onBettingRoundAnimation(myBettingRoundID);
            }
        }
    }
}

bool BettingRound::allBetsAreDone() const
{
    PlayerListIterator itC;

    for (itC = myHand->getRunningPlayersList()->begin(); itC != myHand->getRunningPlayersList()->end(); ++itC)
    {
        if (myHighestSet != (*itC)->getSet())
        {
            return false;
        }
    }

    return true;
}

void BettingRound::proceedToNextBettingRound()
{
    PlayerListIterator itC;
    myHand->setCurrentRoundState(GameState(myBettingRoundID + 1));

    for (itC = myHand->getRunningPlayersList()->begin(); itC != myHand->getRunningPlayersList()->end(); ++itC)
    {
        (*itC)->setAction(PlayerActionNone);
    }

    myHand->getBoard()->collectSets();
    myHand->getBoard()->collectPot();

    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(myHand->getBoard()->getPot());
    }

    if (myEvents.onRefreshSet)
    {
        myEvents.onRefreshSet();
    }

    if (myEvents.onRefreshCash)
    {
        myEvents.onRefreshCash();
    }

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        if (myEvents.onRefreshAction)
        {
            myEvents.onRefreshAction(i, PlayerActionNone);
        }
    }

    myHand->resolveHandConditions();
}

void BettingRound::logBoardCards()
{
    if (!myLogBoardCardsDone)
    {
        int tempBoardCardsArray[5];
        myHand->getBoard()->getCards(tempBoardCardsArray);

        switch (myBettingRoundID)
        {
        case GameStateFlop:
            GlobalServices::instance().logger()->info(
                "\n\n************************* FLOP " + CardUtilities::getCardString(tempBoardCardsArray[0]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[1]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[2]) + "  *************************\n\n");
            break;
        case GameStateTurn:
            GlobalServices::instance().logger()->info(
                "\n\n************************* TURN " + CardUtilities::getCardString(tempBoardCardsArray[0]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[1]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[2]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[3]) + "  *************************\n\n");

            break;
        case GameStateRiver:
            GlobalServices::instance().logger()->info(
                "\n\n************************* RIVER " + CardUtilities::getCardString(tempBoardCardsArray[0]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[1]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[2]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[3]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[4]) + "  *************************\n\n");

            break;
        default:
            GlobalServices::instance().logger()->error("wrong myBettingRoundID");
        }
        myLogBoardCardsDone = true;
    }
}

void BettingRound::handleFirstRunGui()
{
    myFirstRunGui = false;
    myHand->setPreviousPlayerId(-1);

    if (myEvents.onDealBettingRoundCards)
    {
        myEvents.onDealBettingRoundCards(myBettingRoundID);
    }
}
void BettingRound::handleFirstRun()
{
    myFirstRun = false;

    if (!(myHand->getAllInCondition()))
    {
        PlayerListIterator it1, it2;

        // running player before smallBlind
        bool formerRunningPlayerFound = false;
        if (myHand->getSeatsList()->size() > 2)
        {
            it1 = myHand->getActivePlayerIt(mySmallBlindPositionId);
            if (it1 == myHand->getSeatsList()->end())
            {
                GlobalServices::instance().logger()->error("ACTIVE_PLAYER_NOT_FOUND");
            }

            for (size_t i = 0; i < myHand->getSeatsList()->size(); i++)
            {
                if (it1 == myHand->getSeatsList()->begin())
                {
                    it1 = myHand->getSeatsList()->end();
                }
                --it1;
                GlobalServices::instance().logger()->verbose("handleFirstRun : finding running player");

                it2 = myHand->getRunningPlayerIt((*it1)->getId());
                // running player found
                if (it2 != myHand->getRunningPlayersList()->end())
                {
                    myFirstRoundLastPlayersTurnId = (*it2)->getId();
                    formerRunningPlayerFound = true;
                    break;
                }
            }
            if (!formerRunningPlayerFound)
            {
                GlobalServices::instance().logger()->error("FORMER_RUNNING_PLAYER_NOT_FOUND");
            }
        }
        // heads up: bigBlind begins -> dealer/smallBlind is running player before bigBlind
        else
        {
            myFirstRoundLastPlayersTurnId = mySmallBlindPositionId;
        }
        myCurrentPlayersTurnId = myFirstRoundLastPlayersTurnId;
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
    myMinimumRaise = theValue;
}
int BettingRound::getMinimumRaise() const
{
    return myMinimumRaise;
}

void BettingRound::setFullBetRule(bool theValue)
{
    myFullBetRule = theValue;
}
bool BettingRound::getFullBetRule() const
{
    return myFullBetRule;
}

void BettingRound::skipFirstRunGui()
{
    myFirstRunGui = false;
}

void BettingRound::postRiverRun() {};

IHand* BettingRound::getHand() const
{
    return myHand;
}

int BettingRound::getDealerPosition() const
{
    return myDealerPosition;
}
void BettingRound::setDealerPosition(int theValue)
{
    myDealerPosition = theValue;
}

void BettingRound::setCurrentPlayersTurnId(unsigned theValue)
{
    myCurrentPlayersTurnId = theValue;
}
unsigned BettingRound::getCurrentPlayersTurnId() const
{
    return myCurrentPlayersTurnId;
}

void BettingRound::setFirstRoundLastPlayersTurnId(unsigned theValue)
{
    myFirstRoundLastPlayersTurnId = theValue;
}
unsigned BettingRound::getFirstRoundLastPlayersTurnId() const
{
    return myFirstRoundLastPlayersTurnId;
}

void BettingRound::setCurrentPlayersTurnIt(PlayerListIterator theValue)
{
    myCurrentPlayersTurnIt = theValue;
}
PlayerListIterator BettingRound::getCurrentPlayersTurnIt() const
{
    return myCurrentPlayersTurnIt;
}

void BettingRound::setLastPlayersTurnIt(PlayerListIterator theValue)
{
    myLastPlayersTurnIt = theValue;
}
PlayerListIterator BettingRound::getLastPlayersTurnIt() const
{
    return myLastPlayersTurnIt;
}

void BettingRound::setHighestSet(int theValue)
{
    myHighestSet = theValue;
}
int BettingRound::getHighestSet() const
{
    return myHighestSet;
}

void BettingRound::setFirstRun(bool theValue)
{
    myFirstRun = theValue;
}
bool BettingRound::getFirstRun() const
{
    return myFirstRun;
}

void BettingRound::setFirstRound(bool theValue)
{
    myFirstRound = theValue;
}
bool BettingRound::getFirstRound() const
{
    return myFirstRound;
}

void BettingRound::setDealerPositionId(unsigned theValue)
{
    myDealerPositionId = theValue;
}
unsigned BettingRound::getDealerPositionId() const
{
    return myDealerPositionId;
}

void BettingRound::setSmallBlindPositionId(unsigned theValue)
{
    mySmallBlindPositionId = theValue;
}
unsigned BettingRound::getSmallBlindPositionId() const
{
    return mySmallBlindPositionId;
}

void BettingRound::setBigBlindPositionId(unsigned theValue)
{
    myBigBlindPositionId = theValue;
}
unsigned BettingRound::getBigBlindPositionId() const
{
    return myBigBlindPositionId;
}

void BettingRound::setSmallBlindPosition(int theValue)
{
    mySmallBlindPosition = theValue;
}
int BettingRound::getSmallBlindPosition() const
{
    return mySmallBlindPosition;
}

void BettingRound::setSmallBlind(int theValue)
{
    mySmallBlind = theValue;
}
int BettingRound::getSmallBlind() const
{
    return mySmallBlind;
}
} // namespace pkt::core
