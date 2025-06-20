// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Hand.h"
#include <core/services/GlobalServices.h>
#include "CardsValue.h"
#include "GameEvents.h"
#include "Randomizer.h"
#include "model/ButtonState.h"

#include "Exception.h"
#include "model/EngineError.h"

#include <algorithm>
#include <iostream>
#include <random>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

Hand::Hand(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
           pkt::core::player::PlayerList seats, PlayerList activePlayers, PlayerList runningPlayers, int handId,
           GameData gameData, StartData startData)
    : myEvents(events), myFactory(factory), myBoard(board), mySeatsList(seats), myActivePlayerList(activePlayers),
      myRunningPlayerList(runningPlayers), myStartQuantityPlayers(startData.numberOfPlayers),
      myDealerPosition(startData.startDealerPlayerId), mySmallBlindPosition(startData.startDealerPlayerId),
      myBigBlindPosition(startData.startDealerPlayerId), myCurrentRound(GameStatePreflop),
      myRoundBeforePostRiver(GameStatePreflop), mySmallBlind(gameData.firstSmallBlind),
      myStartCash(gameData.startMoney), myPreviousPlayerId(-1), myLastActionPlayerId(0), myAllInCondition(false),
      myCardsShown(false)
{

    GlobalServices::instance().logger()->info("\n-------------------------------------------------------------\n");
    GlobalServices::instance().logger()->info("\nHAND " + std::to_string(myID) + "\n");

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->setHand(this);
        (*it)->setCardsFlip(0);
    }

    initAndShuffleDeck();
    size_t cardsArrayIndex = dealBoardCards(); // we need to deal the board first,
                                               // so that the players can use it to evaluate their hands
    dealHoleCards(cardsArrayIndex);

    myPreflopLastRaiserId = -1;

    // determine dealer, SB, BB
    assignButtons();

    setBlinds();

    myBettingRound = myFactory->createBettingRound(this, myDealerPosition, mySmallBlind);
}

Hand::~Hand()
{
}

void Hand::initAndShuffleDeck()
{
    myCardsArray = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 rng(rd()); // Seed the Mersenne Twister random number generator
    std::shuffle(myCardsArray.begin(), myCardsArray.end(), rng);
}

void Hand::dealHoleCards(size_t cardsArrayIndex)
{
    int i, j, k = 0;
    int tempPlayerArray[2];
    int tempPlayerAndBoardArray[7];

    // init the first 5 cards of the board
    // NB. cardsArray has already 5 board cards
    for (i = 0; i < 5; i++)
    {
        tempPlayerAndBoardArray[i] = myCardsArray[i];
    }

    for (auto it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it, k++)
    {
        for (j = 0; j < 2; j++)
        {
            tempPlayerArray[j] = myCardsArray[2 * k + j + 5];
            tempPlayerAndBoardArray[j] = myCardsArray[2 * k + j + 5];
        }

        (*it)->setCards(tempPlayerArray);
        (*it)->setCardsValueInt(CardsValue::evaluateHand(tempPlayerAndBoardArray));
        (*it)->setRoundStartCash((*it)->getCash());
        (*it)->getCurrentHandActions().reset();
        (*it)->setPosition();
        (*it)->getRangeEstimator()->setEstimatedRange("");
    }
}
size_t Hand::dealBoardCards()
{
    int tempBoardArray[5];
    size_t cardsArrayIndex = 0;

    for (size_t i = 0; i < 5; ++i) // The board consists of 5 cards (Flop, Turn, River)
    {
        tempBoardArray[i] = myCardsArray[cardsArrayIndex++];
    }

    myBoard->setCards(tempBoardArray);
    return cardsArrayIndex;
}

void Hand::start()
{
    if (myEvents.onDealHoleCards)
        myEvents.onDealHoleCards();

    getBoard()->collectSets();

    if (myEvents.onPotUpdated)
        myEvents.onPotUpdated(getBoard()->getPot());

    if (myEvents.onActivePlayerActionDone)
        myEvents.onActivePlayerActionDone();
}

void Hand::assignButtons()
{

    size_t i;
    PlayerListIterator it;
    PlayerListConstIterator it_c;

    // delete all buttons
    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->setButton(ButtonNone);
    }

    // assign dealer button
    it = getSeatIt(myDealerPosition);
    if (it == mySeatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }
    (*it)->setButton(ButtonDealer);

    // assign Small Blind next to dealer. ATTENTION: in heads up it is big blind
    // assign big blind next to small blind. ATTENTION: in heads up it is small blind
    bool nextActivePlayerFound = false;
    PlayerListIterator dealerPositionIt = getSeatIt(myDealerPosition);
    if (dealerPositionIt == mySeatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }

    for (i = 0; i < mySeatsList->size(); i++)
    {

        ++dealerPositionIt;
        if (dealerPositionIt == mySeatsList->end())
            dealerPositionIt = mySeatsList->begin();

        it = getActivePlayerIt((*dealerPositionIt)->getId());
        if (it != myActivePlayerList->end())
        {
            nextActivePlayerFound = true;
            if (myActivePlayerList->size() > 2)
            {
                // small blind normal
                (*it)->setButton(2);
                mySmallBlindPosition = (*it)->getId();
            }
            else
            {
                // big blind in heads up
                (*it)->setButton(3);
                myBigBlindPosition = (*it)->getId();
                // lastPlayerAction for showing cards
            }

            // first player after dealer have to show his cards first (in showdown)
            myLastActionPlayerId = (*it)->getId();
            myBoard->setLastActionPlayerId(myLastActionPlayerId);

            ++it;
            if (it == myActivePlayerList->end())
                it = myActivePlayerList->begin();

            if (myActivePlayerList->size() > 2)
            {
                // big blind normal
                (*it)->setButton(3);
                myBigBlindPosition = (*it)->getId();
            }
            else
            {
                // small blind in heads up
                (*it)->setButton(2);
                mySmallBlindPosition = (*it)->getId();
            }

            break;
        }
    }
    if (!nextActivePlayerFound)
    {
        throw Exception(__FILE__, __LINE__, EngineError::NextActivePlayerNotFound);
    }
}

void Hand::setBlinds()
{

    PlayerListConstIterator it_c;

    // do sets --> TODO switch?
    for (it_c = myRunningPlayerList->begin(); it_c != myRunningPlayerList->end(); ++it_c)
    {

        // small blind
        if ((*it_c)->getButton() == ButtonSmallBlind)
        {

            // All in ?
            if ((*it_c)->getCash() <= mySmallBlind)
            {

                (*it_c)->setSet((*it_c)->getCash());
                // 1 to do not log this
                (*it_c)->setAction(PlayerActionAllin, 1);
            }
            else
            {
                (*it_c)->setSet(mySmallBlind);
            }
        }
    }

    // do sets --> TODO switch?
    for (it_c = myRunningPlayerList->begin(); it_c != myRunningPlayerList->end(); ++it_c)
    {

        // big blind
        if ((*it_c)->getButton() == ButtonBigBlind)
        {

            // all in ?
            if ((*it_c)->getCash() <= 2 * mySmallBlind)
            {

                (*it_c)->setSet((*it_c)->getCash());
                // 1 to do not log this
                (*it_c)->setAction(PlayerActionAllin, 1);
            }
            else
            {
                (*it_c)->setSet(2 * mySmallBlind);
            }
        }
    }
}

void Hand::switchRounds()
{

    PlayerListIterator it, it_1;
    PlayerListConstIterator it_c;

    // refresh runningPlayerList
    for (it = myRunningPlayerList->begin(); it != myRunningPlayerList->end();)
    {
        if ((*it)->getAction() == PlayerActionFold || (*it)->getAction() == PlayerActionAllin)
        {

            it = myRunningPlayerList->erase(it);
            if (!(myRunningPlayerList->empty()))
            {

                it_1 = it;
                if (it_1 == myRunningPlayerList->begin())
                    it_1 = myRunningPlayerList->end();
                --it_1;
                getCurrentBettingRound()->setCurrentPlayersTurnId((*it_1)->getId());
            }
        }
        else
        {
            ++it;
        }
    }

    // determine number of all in players
    int allInPlayersCounter = 0;
    for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() == PlayerActionAllin)
            allInPlayersCounter++;
    }

    // determine number of non-fold players
    int nonFoldPlayerCounter = 0;
    for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PlayerActionFold)
            nonFoldPlayerCounter++;
    }

    // if only one player non-fold -> distribute pot
    if (nonFoldPlayerCounter == 1)
    {
        myBoard->collectPot();
        if (myEvents.onPotUpdated)
            myEvents.onPotUpdated(myBoard->getPot());

        if (myEvents.onRefreshSet)
            myEvents.onRefreshSet();
        ;
        myCurrentRound = GameStatePostRiver;
    }

    // check for all in condition
    // for all in condition at least two active players have to remain
    else
    {

        // 1) all players all in
        if (allInPlayersCounter == nonFoldPlayerCounter)
        {
            myAllInCondition = true;
            myBoard->setAllInCondition(true);
        }

        // 2) all players but one all in and he has highest set
        if (allInPlayersCounter + 1 == nonFoldPlayerCounter)
        {

            for (it_c = myRunningPlayerList->begin(); it_c != myRunningPlayerList->end(); ++it_c)
            {

                if ((*it_c)->getSet() >= myBettingRound[myCurrentRound]->getHighestSet())
                {
                    myAllInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }

            // exception
            // no.1: if in first Preflop Round next player is small blind and only all-in-big-blind with less than
            // smallblind amount and other all-in players with less than small blind are nonfold too -> preflop is over
            PlayerListConstIterator smallBlindIt_c =
                getRunningPlayerIt(myBettingRound[myCurrentRound]->getSmallBlindPositionId());
            PlayerListConstIterator bigBlindIt_c =
                getActivePlayerIt(myBettingRound[myCurrentRound]->getBigBlindPositionId());
            if (smallBlindIt_c != myRunningPlayerList->end() && bigBlindIt_c != myActivePlayerList->end() &&
                myCurrentRound == GameStatePreflop && myBettingRound[myCurrentRound]->getFirstRound())
            {
                // determine player who are all in with less than small blind amount
                int tempCounter = 0;
                for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
                {
                    if ((*it_c)->getAction() == PlayerActionAllin && (*it_c)->getSet() <= mySmallBlind)
                    {
                        tempCounter++;
                    }
                }
                if ((*bigBlindIt_c)->getSet() <= mySmallBlind && tempCounter == allInPlayersCounter)
                {
                    myAllInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }

            // no.2: heads up -> detect player who is all in and bb but could set less than sb
            for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
            {

                if (myActivePlayerList->size() == 2 && (*it_c)->getAction() == PlayerActionAllin &&
                    (*it_c)->getButton() == ButtonBigBlind && (*it_c)->getSet() <= mySmallBlind &&
                    myCurrentRound == GameStatePreflop)
                {
                    myAllInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }
        }
    }

    // special routine
    if (myAllInCondition)
    {
        myBoard->collectPot();

        if (myEvents.onPotUpdated)
            myEvents.onPotUpdated(myBoard->getPot());

        if (myEvents.onRefreshSet)
            myEvents.onRefreshSet();

        if (myEvents.onFlipHoleCardsAllIn)
            myEvents.onFlipHoleCardsAllIn();

        if (myCurrentRound < GameStatePostRiver) // do not increment past 4
        {
            myCurrentRound = GameState(myCurrentRound + 1);
        }
        { // do not increment past 4
            myCurrentRound = GameState(myCurrentRound + 1);
        }

        // log board cards for allin
        if (myCurrentRound >= GameStateFlop)
        {
            int tempBoardCardsArray[5];

            myBoard->getCards(tempBoardCardsArray);
        }
    }

    // unhighlight current players groupbox
    it_c = getActivePlayerIt(myPreviousPlayerId);
    if (it_c != myActivePlayerList->end())
    {
        // lastPlayersTurn is active
        if (myEvents.onRefreshPlayersActiveInactiveStyles)
            myEvents.onRefreshPlayersActiveInactiveStyles(myPreviousPlayerId, 1);
    }

    if (myEvents.onRefreshTableDescriptiveLabels)
        myEvents.onRefreshTableDescriptiveLabels(getCurrentRound());

    if (myCurrentRound < GameStatePostRiver)
    {
        myRoundBeforePostRiver = myCurrentRound;
    }

    switch (myCurrentRound)
    {
    case GameStatePreflop:
    {
        if (myEvents.onPreflopAnimation)
            myEvents.onPreflopAnimation();
    }
    break;
    case GameStateFlop:
    {
        if (myEvents.onFlopAnimation)
            myEvents.onFlopAnimation();
    }
    break;
    case GameStateTurn:
    {
        if (myEvents.onTurnAnimation)
            myEvents.onTurnAnimation();
    }
    break;
    case GameStateRiver:
    {
        if (myEvents.onRiverAnimation)
            myEvents.onRiverAnimation();
    }
    break;
    case GameStatePostRiver:
    {
        if (myEvents.onPostRiverAnimation)
            myEvents.onPostRiverAnimation();
    }
    break;
    default:
    {
    }
    }
}

PlayerListIterator Hand::getSeatIt(unsigned uniqueId) const
{

    PlayerListIterator it;

    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        if ((*it)->getId() == uniqueId)
        {
            break;
        }
    }

    return it;
}

PlayerListIterator Hand::getActivePlayerIt(unsigned uniqueId) const
{

    PlayerListIterator it;

    for (it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it)
    {
        if ((*it)->getId() == uniqueId)
        {
            break;
        }
    }

    return it;
}

PlayerListIterator Hand::getRunningPlayerIt(unsigned uniqueId) const
{

    PlayerListIterator it;

    for (it = myRunningPlayerList->begin(); it != myRunningPlayerList->end(); ++it)
    {

        if ((*it)->getId() == uniqueId)
        {
            break;
        }
    }

    return it;
}

void Hand::setLastActionPlayerId(unsigned theValue)
{
    myLastActionPlayerId = theValue;
    myBoard->setLastActionPlayerId(theValue);
}

int Hand::getPreflopCallsNumber()
{

    int calls = 0;

    for (PlayerListIterator it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getPreflopActions();

        if (find(actions.begin(), actions.end(), PlayerActionCall) != actions.end())
            calls++;
    }
    return calls;
}
int Hand::getPreflopRaisesNumber()
{

    int raises = 0;

    for (PlayerListIterator it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getPreflopActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin)
                raises++;
    }

    return raises;
}
int Hand::getFlopBetsOrRaisesNumber()
{

    int bets = 0;

    for (PlayerListIterator it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getFlopActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin || (*itAction) == PlayerActionBet)
                bets++;
    }

    return bets;
}
int Hand::getTurnBetsOrRaisesNumber()
{

    int bets = 0;

    for (PlayerListIterator it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getTurnActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin || (*itAction) == PlayerActionBet)
                bets++;
    }

    return bets;
}
int Hand::getRiverBetsOrRaisesNumber()
{

    int bets = 0;

    for (PlayerListIterator it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getRiverActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin || (*itAction) == PlayerActionBet)
                bets++;
    }

    return bets;
}
std::vector<PlayerPosition> Hand::getRaisersPositions()
{

    std::vector<PlayerPosition> positions;

    PlayerListIterator it_c;

    for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
    { // note that all in players are not "running" any more

        if ((*it_c)->getAction() == PlayerActionRaise || (*it_c)->getAction() == PlayerActionAllin)
            positions.push_back((*it_c)->getPosition());
    }
    return positions;
}

std::vector<PlayerPosition> Hand::getCallersPositions()
{

    std::vector<PlayerPosition> positions;

    PlayerListIterator it_c;

    for (it_c = myRunningPlayerList->begin(); it_c != myRunningPlayerList->end(); ++it_c)
    {

        if ((*it_c)->getAction() == PlayerActionCall)
            positions.push_back((*it_c)->getPosition());
    }
    return positions;
}
int Hand::getLastRaiserId()
{

    PlayerListIterator lastRaiser = myActivePlayerList->end();

    PlayerList players = myActivePlayerList;

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getAction() == PlayerActionRaise || (*it)->getAction() == PlayerActionAllin)
        {

            if (lastRaiser != myActivePlayerList->end())
            {
                if ((*lastRaiser)->getPosition() < (*it)->getPosition())
                    lastRaiser = it;
            }
            else
            {
                lastRaiser = it;
            }
        }
    }
    if (lastRaiser != myActivePlayerList->end())
        return (*lastRaiser)->getId();

    // if no raiser was found, look for the one who have bet (if any)

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getAction() == PlayerActionBet)
            lastRaiser = it;
    }
    if (lastRaiser != myActivePlayerList->end())
        return (*lastRaiser)->getId();
    else
        return -1;
}
int Hand::getPreflopLastRaiserId()
{
    return myPreflopLastRaiserId;
}

void Hand::setPreflopLastRaiserId(int id)
{
    myPreflopLastRaiserId = id;
}
int Hand::getFlopLastRaiserId()
{
    return myFlopLastRaiserId;
}

void Hand::setFlopLastRaiserId(int id)
{
    myFlopLastRaiserId = id;
}
int Hand::getTurnLastRaiserId()
{
    return myTurnLastRaiserId;
}

void Hand::setTurnLastRaiserId(int id)
{
    myTurnLastRaiserId = id;
}
} // namespace pkt::core
