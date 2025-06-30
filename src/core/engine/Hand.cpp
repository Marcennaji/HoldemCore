// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Hand.h"
#include <core/services/GlobalServices.h>
#include "CardUtilities.h"
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
      myBigBlindPosition(startData.startDealerPlayerId), mySmallBlind(gameData.firstSmallBlind),
      myStartCash(gameData.startMoney)

{

    GlobalServices::instance().logger()->info("\n-------------------------------------------------------------\n");

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

    myBettingRounds = myFactory->createAllBettingRounds(this, myDealerPosition, mySmallBlind);

    // Initialize FSM - start with preflop state
    // This will be expanded in Phase 2 to use the factory pattern
    // For now, we create skeleton state for compilation
}

Hand::~Hand() = default;

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
    int boardCardIndex, holeCardIndex, playerIndex = 0;
    int tempPlayerArray[2];
    int tempPlayerAndBoardArray[7];

    // Validate that there are enough cards in the deck
    if (myCardsArray.size() < 5 + 2 * myActivePlayerList->size())
    {
        throw std::runtime_error("Not enough cards in the deck to deal hole cards and board cards.");
    }

    // Initialize the first 5 cards of the board
    for (boardCardIndex = 0; boardCardIndex < 5; boardCardIndex++)
    {
        tempPlayerAndBoardArray[boardCardIndex] = myCardsArray[boardCardIndex];
    }

    for (auto it = myActivePlayerList->begin(); it != myActivePlayerList->end(); ++it, playerIndex++)
    {
        for (holeCardIndex = 0; holeCardIndex < 2; holeCardIndex++)
        {
            tempPlayerArray[holeCardIndex] = myCardsArray[2 * playerIndex + holeCardIndex + 5];
            tempPlayerAndBoardArray[5 + holeCardIndex] = myCardsArray[2 * playerIndex + holeCardIndex + 5];
        }
        string humanReadableHand = CardUtilities::getCardStringValue(tempPlayerAndBoardArray, 7);
        (*it)->setCards(tempPlayerArray);
        (*it)->setHandRanking(HandEvaluator::evaluateHand(humanReadableHand.c_str()));
        (*it)->setRoundStartCash((*it)->getCash());
        (*it)->getCurrentHandActions().reset();
        (*it)->setPosition();
        (*it)->getRangeEstimator()->setEstimatedRange("");
        GlobalServices::instance().logger()->info("Player " + (*it)->getName() +
                                                  " dealt cards: " + CardUtilities::getCardString(tempPlayerArray[0]) +
                                                  " " + CardUtilities::getCardString(tempPlayerArray[1]) +
                                                  ", hand strength = " + std::to_string((*it)->getCardsValueInt()));
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
    {
        myEvents.onDealHoleCards();
    }

    getBoard()->collectSets();

    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(getBoard()->getPot());
    }

    if (myEvents.onActivePlayerActionDone)
    {
        myEvents.onActivePlayerActionDone();
    }
}

void Hand::assignButtons()
{

    size_t i;
    PlayerListIterator it;
    PlayerListConstIterator itC;

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
        {
            dealerPositionIt = mySeatsList->begin();
        }

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
            {
                it = myActivePlayerList->begin();
            }

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

    PlayerListConstIterator itC;

    // do sets --> TODO switch?
    for (itC = myRunningPlayerList->begin(); itC != myRunningPlayerList->end(); ++itC)
    {

        // small blind
        if ((*itC)->getButton() == ButtonSmallBlind)
        {

            // All in ?
            if ((*itC)->getCash() <= mySmallBlind)
            {

                (*itC)->setSet((*itC)->getCash());
                // 1 to do not log this
                (*itC)->setAction(PlayerActionAllin, 1);
            }
            else
            {
                (*itC)->setSet(mySmallBlind);
            }
        }
    }

    // do sets --> TODO switch?
    for (itC = myRunningPlayerList->begin(); itC != myRunningPlayerList->end(); ++itC)
    {

        // big blind
        if ((*itC)->getButton() == ButtonBigBlind)
        {

            // all in ?
            if ((*itC)->getCash() <= 2 * mySmallBlind)
            {

                (*itC)->setSet((*itC)->getCash());
                // 1 to do not log this
                (*itC)->setAction(PlayerActionAllin, 1);
            }
            else
            {
                (*itC)->setSet(2 * mySmallBlind);
            }
        }
    }
}

void Hand::switchRounds()
{

    PlayerListIterator it, it1;
    PlayerListConstIterator itC;

    // refresh runningPlayerList
    for (it = myRunningPlayerList->begin(); it != myRunningPlayerList->end();)
    {
        if ((*it)->getAction() == PlayerActionFold || (*it)->getAction() == PlayerActionAllin)
        {

            it = myRunningPlayerList->erase(it);
            if (!(myRunningPlayerList->empty()))
            {

                it1 = it;
                if (it1 == myRunningPlayerList->begin())
                {
                    it1 = myRunningPlayerList->end();
                }
                --it1;
                getCurrentBettingRound()->setCurrentPlayersTurnId((*it1)->getId());
            }
        }
        else
        {
            ++it;
        }
    }

    // determine number of all in players
    int allInPlayersCounter = 0;
    for (itC = myActivePlayerList->begin(); itC != myActivePlayerList->end(); ++itC)
    {
        if ((*itC)->getAction() == PlayerActionAllin)
        {
            allInPlayersCounter++;
        }
    }

    // determine number of non-fold players
    int nonFoldPlayerCounter = 0;
    for (itC = myActivePlayerList->begin(); itC != myActivePlayerList->end(); ++itC)
    {
        if ((*itC)->getAction() != PlayerActionFold)
        {
            nonFoldPlayerCounter++;
        }
    }

    // if only one player non-fold -> distribute pot
    if (nonFoldPlayerCounter == 1)
    {
        myBoard->collectPot();
        if (myEvents.onPotUpdated)
        {
            myEvents.onPotUpdated(myBoard->getPot());
        }

        if (myEvents.onRefreshSet)
        {
            myEvents.onRefreshSet();
        };
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

            for (itC = myRunningPlayerList->begin(); itC != myRunningPlayerList->end(); ++itC)
            {

                if ((*itC)->getSet() >= myBettingRounds[myCurrentRound]->getHighestSet())
                {
                    myAllInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }

            // exception
            // no.1: if in first Preflop Round next player is small blind and only all-in-big-blind with less than
            // smallblind amount and other all-in players with less than small blind are nonfold too -> preflop is over
            PlayerListConstIterator smallBlindItC =
                getRunningPlayerIt(myBettingRounds[myCurrentRound]->getSmallBlindPositionId());
            PlayerListConstIterator bigBlindItC =
                getActivePlayerIt(myBettingRounds[myCurrentRound]->getBigBlindPositionId());
            if (smallBlindItC != myRunningPlayerList->end() && bigBlindItC != myActivePlayerList->end() &&
                myCurrentRound == GameStatePreflop && myBettingRounds[myCurrentRound]->getFirstRound())
            {
                // determine player who are all in with less than small blind amount
                int tempCounter = 0;
                for (itC = myActivePlayerList->begin(); itC != myActivePlayerList->end(); ++itC)
                {
                    if ((*itC)->getAction() == PlayerActionAllin && (*itC)->getSet() <= mySmallBlind)
                    {
                        tempCounter++;
                    }
                }
                if ((*bigBlindItC)->getSet() <= mySmallBlind && tempCounter == allInPlayersCounter)
                {
                    myAllInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }

            // no.2: heads up -> detect player who is all in and bb but could set less than sb
            for (itC = myActivePlayerList->begin(); itC != myActivePlayerList->end(); ++itC)
            {

                if (myActivePlayerList->size() == 2 && (*itC)->getAction() == PlayerActionAllin &&
                    (*itC)->getButton() == ButtonBigBlind && (*itC)->getSet() <= mySmallBlind &&
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
        {
            myEvents.onPotUpdated(myBoard->getPot());
        }

        if (myEvents.onRefreshSet)
        {
            myEvents.onRefreshSet();
        }

        if (myEvents.onFlipHoleCardsAllIn)
        {
            myEvents.onFlipHoleCardsAllIn();
        }

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
    itC = getActivePlayerIt(myPreviousPlayerId);
    if (itC != myActivePlayerList->end())
    {
        // lastPlayersTurn is active
        if (myEvents.onRefreshPlayersActiveInactiveStyles)
        {
            myEvents.onRefreshPlayersActiveInactiveStyles(myPreviousPlayerId, 1);
        }
    }

    if (myEvents.onRefreshTableDescriptiveLabels)
    {
        myEvents.onRefreshTableDescriptiveLabels(getCurrentRound());
    }

    if (myCurrentRound < GameStatePostRiver)
    {
        myRoundBeforePostRiver = myCurrentRound;
    }

    switch (myCurrentRound)
    {
    case GameStatePreflop:
    {
        if (myEvents.onStartPreflop)
        {
            myEvents.onStartPreflop();
        }
    }
    break;
    case GameStateFlop:
    {
        if (myEvents.onStartFlop)
        {
            myEvents.onStartFlop();
        }
    }
    break;
    case GameStateTurn:
    {
        if (myEvents.onStartTurn)
        {
            myEvents.onStartTurn();
        }
    }
    break;
    case GameStateRiver:
    {
        if (myEvents.onStartRiver)
        {
            myEvents.onStartRiver();
        }
    }
    break;
    case GameStatePostRiver:
    {
        if (myEvents.onStartPostRiver)
        {
            myEvents.onStartPostRiver();
        }
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
        {
            calls++;
        }
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
        {
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin)
            {
                raises++;
            }
        }
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
        {
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin || (*itAction) == PlayerActionBet)
            {
                bets++;
            }
        }
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
        {
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin || (*itAction) == PlayerActionBet)
            {
                bets++;
            }
        }
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
        {
            if ((*itAction) == PlayerActionRaise || (*itAction) == PlayerActionAllin || (*itAction) == PlayerActionBet)
            {
                bets++;
            }
        }
    }

    return bets;
}
std::vector<PlayerPosition> Hand::getRaisersPositions()
{

    std::vector<PlayerPosition> positions;

    PlayerListIterator itC;

    for (itC = myActivePlayerList->begin(); itC != myActivePlayerList->end(); ++itC)
    { // note that all in players are not "running" any more

        if ((*itC)->getAction() == PlayerActionRaise || (*itC)->getAction() == PlayerActionAllin)
        {
            positions.push_back((*itC)->getPosition());
        }
    }
    return positions;
}

std::vector<PlayerPosition> Hand::getCallersPositions()
{

    std::vector<PlayerPosition> positions;

    PlayerListIterator itC;

    for (itC = myRunningPlayerList->begin(); itC != myRunningPlayerList->end(); ++itC)
    {

        if ((*itC)->getAction() == PlayerActionCall)
        {
            positions.push_back((*itC)->getPosition());
        }
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
                {
                    lastRaiser = it;
                }
            }
            else
            {
                lastRaiser = it;
            }
        }
    }
    if (lastRaiser != myActivePlayerList->end())
    {
        return (*lastRaiser)->getId();
    }

    // if no raiser was found, look for the one who have bet (if any)

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getAction() == PlayerActionBet)
        {
            lastRaiser = it;
        }
    }
    if (lastRaiser != myActivePlayerList->end())
    {
        return (*lastRaiser)->getId();
    }
    else
    {
        return -1;
    }
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
