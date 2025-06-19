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
    : myEvents(events), myFactory(factory), myBoard(board), seatsList(seats), activePlayerList(activePlayers),
      runningPlayerList(runningPlayers), startQuantityPlayers(startData.numberOfPlayers),
      dealerPosition(startData.startDealerPlayerId), smallBlindPosition(startData.startDealerPlayerId),
      bigBlindPosition(startData.startDealerPlayerId), currentRound(GAME_STATE_PREFLOP),
      roundBeforePostRiver(GAME_STATE_PREFLOP), smallBlind(gameData.firstSmallBlind), startCash(gameData.startMoney),
      previousPlayerID(-1), lastActionPlayerID(0), allInCondition(false), cardsShown(false)
{

    GlobalServices::instance().logger()->info("\n-------------------------------------------------------------\n");
    GlobalServices::instance().logger()->info("\nHAND " + std::to_string(myID) + "\n");

    for (auto it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        (*it)->setHand(this);
        (*it)->setCardsFlip(0);
    }

    initAndShuffleDeck();
    size_t cardsArrayIndex = dealBoardCards(); // we need to deal the board first,
                                               // so that the players can use it to evaluate their hands
    dealHoleCards(cardsArrayIndex);

    preflopLastRaiserID = -1;

    // determine dealer, SB, BB
    assignButtons();

    setBlinds();

    myBettingRound = myFactory->createBettingRound(this, dealerPosition, smallBlind);
}

Hand::~Hand()
{
}

void Hand::initAndShuffleDeck()
{
    cardsArray = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 rng(rd()); // Seed the Mersenne Twister random number generator
    std::shuffle(cardsArray.begin(), cardsArray.end(), rng);
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
        tempPlayerAndBoardArray[i] = cardsArray[i];
    }

    for (auto it = activePlayerList->begin(); it != activePlayerList->end(); ++it, k++)
    {
        for (j = 0; j < 2; j++)
        {
            tempPlayerArray[j] = cardsArray[2 * k + j + 5];
            tempPlayerAndBoardArray[j] = cardsArray[2 * k + j + 5];
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
        tempBoardArray[i] = cardsArray[cardsArrayIndex++];
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
    for (it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        (*it)->setButton(BUTTON_NONE);
    }

    // assign dealer button
    it = getSeatIt(dealerPosition);
    if (it == seatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SEAT_NOT_FOUND);
    }
    (*it)->setButton(BUTTON_DEALER);

    // assign Small Blind next to dealer. ATTENTION: in heads up it is big blind
    // assign big blind next to small blind. ATTENTION: in heads up it is small blind
    bool nextActivePlayerFound = false;
    PlayerListIterator dealerPositionIt = getSeatIt(dealerPosition);
    if (dealerPositionIt == seatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SEAT_NOT_FOUND);
    }

    for (i = 0; i < seatsList->size(); i++)
    {

        ++dealerPositionIt;
        if (dealerPositionIt == seatsList->end())
            dealerPositionIt = seatsList->begin();

        it = getActivePlayerIt((*dealerPositionIt)->getID());
        if (it != activePlayerList->end())
        {
            nextActivePlayerFound = true;
            if (activePlayerList->size() > 2)
            {
                // small blind normal
                (*it)->setButton(2);
                smallBlindPosition = (*it)->getID();
            }
            else
            {
                // big blind in heads up
                (*it)->setButton(3);
                bigBlindPosition = (*it)->getID();
                // lastPlayerAction for showing cards
            }

            // first player after dealer have to show his cards first (in showdown)
            lastActionPlayerID = (*it)->getID();
            myBoard->setLastActionPlayerID(lastActionPlayerID);

            ++it;
            if (it == activePlayerList->end())
                it = activePlayerList->begin();

            if (activePlayerList->size() > 2)
            {
                // big blind normal
                (*it)->setButton(3);
                bigBlindPosition = (*it)->getID();
            }
            else
            {
                // small blind in heads up
                (*it)->setButton(2);
                smallBlindPosition = (*it)->getID();
            }

            break;
        }
    }
    if (!nextActivePlayerFound)
    {
        throw Exception(__FILE__, __LINE__, EngineError::NEXT_ACTIVE_PLAYER_NOT_FOUND);
    }
}

void Hand::setBlinds()
{

    PlayerListConstIterator it_c;

    // do sets --> TODO switch?
    for (it_c = runningPlayerList->begin(); it_c != runningPlayerList->end(); ++it_c)
    {

        // small blind
        if ((*it_c)->getButton() == BUTTON_SMALL_BLIND)
        {

            // All in ?
            if ((*it_c)->getCash() <= smallBlind)
            {

                (*it_c)->setSet((*it_c)->getCash());
                // 1 to do not log this
                (*it_c)->setAction(PLAYER_ACTION_ALLIN, 1);
            }
            else
            {
                (*it_c)->setSet(smallBlind);
            }
        }
    }

    // do sets --> TODO switch?
    for (it_c = runningPlayerList->begin(); it_c != runningPlayerList->end(); ++it_c)
    {

        // big blind
        if ((*it_c)->getButton() == BUTTON_BIG_BLIND)
        {

            // all in ?
            if ((*it_c)->getCash() <= 2 * smallBlind)
            {

                (*it_c)->setSet((*it_c)->getCash());
                // 1 to do not log this
                (*it_c)->setAction(PLAYER_ACTION_ALLIN, 1);
            }
            else
            {
                (*it_c)->setSet(2 * smallBlind);
            }
        }
    }
}

void Hand::switchRounds()
{

    PlayerListIterator it, it_1;
    PlayerListConstIterator it_c;

    // refresh runningPlayerList
    for (it = runningPlayerList->begin(); it != runningPlayerList->end();)
    {
        if ((*it)->getAction() == PLAYER_ACTION_FOLD || (*it)->getAction() == PLAYER_ACTION_ALLIN)
        {

            it = runningPlayerList->erase(it);
            if (!(runningPlayerList->empty()))
            {

                it_1 = it;
                if (it_1 == runningPlayerList->begin())
                    it_1 = runningPlayerList->end();
                --it_1;
                getCurrentBettingRound()->setCurrentPlayersTurnId((*it_1)->getID());
            }
        }
        else
        {
            ++it;
        }
    }

    // determine number of all in players
    int allInPlayersCounter = 0;
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() == PLAYER_ACTION_ALLIN)
            allInPlayersCounter++;
    }

    // determine number of non-fold players
    int nonFoldPlayerCounter = 0;
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
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
        currentRound = GAME_STATE_POST_RIVER;
    }

    // check for all in condition
    // for all in condition at least two active players have to remain
    else
    {

        // 1) all players all in
        if (allInPlayersCounter == nonFoldPlayerCounter)
        {
            allInCondition = true;
            myBoard->setAllInCondition(true);
        }

        // 2) all players but one all in and he has highest set
        if (allInPlayersCounter + 1 == nonFoldPlayerCounter)
        {

            for (it_c = runningPlayerList->begin(); it_c != runningPlayerList->end(); ++it_c)
            {

                if ((*it_c)->getSet() >= myBettingRound[currentRound]->getHighestSet())
                {
                    allInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }

            // exception
            // no.1: if in first Preflop Round next player is small blind and only all-in-big-blind with less than
            // smallblind amount and other all-in players with less than small blind are nonfold too -> preflop is over
            PlayerListConstIterator smallBlindIt_c =
                getRunningPlayerIt(myBettingRound[currentRound]->getSmallBlindPositionId());
            PlayerListConstIterator bigBlindIt_c =
                getActivePlayerIt(myBettingRound[currentRound]->getBigBlindPositionId());
            if (smallBlindIt_c != runningPlayerList->end() && bigBlindIt_c != activePlayerList->end() &&
                currentRound == GAME_STATE_PREFLOP && myBettingRound[currentRound]->getFirstRound())
            {
                // determine player who are all in with less than small blind amount
                int tempCounter = 0;
                for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
                {
                    if ((*it_c)->getAction() == PLAYER_ACTION_ALLIN && (*it_c)->getSet() <= smallBlind)
                    {
                        tempCounter++;
                    }
                }
                if ((*bigBlindIt_c)->getSet() <= smallBlind && tempCounter == allInPlayersCounter)
                {
                    allInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }

            // no.2: heads up -> detect player who is all in and bb but could set less than sb
            for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
            {

                if (activePlayerList->size() == 2 && (*it_c)->getAction() == PLAYER_ACTION_ALLIN &&
                    (*it_c)->getButton() == BUTTON_BIG_BLIND && (*it_c)->getSet() <= smallBlind &&
                    currentRound == GAME_STATE_PREFLOP)
                {
                    allInCondition = true;
                    myBoard->setAllInCondition(true);
                }
            }
        }
    }

    // special routine
    if (allInCondition)
    {
        myBoard->collectPot();

        if (myEvents.onPotUpdated)
            myEvents.onPotUpdated(myBoard->getPot());

        if (myEvents.onRefreshSet)
            myEvents.onRefreshSet();

        if (myEvents.onFlipHoleCardsAllIn)
            myEvents.onFlipHoleCardsAllIn();

        if (currentRound < GAME_STATE_POST_RIVER)
        { // do not increment past 4
            currentRound = GameState(currentRound + 1);
        }

        // log board cards for allin
        if (currentRound >= GAME_STATE_FLOP)
        {
            int tempBoardCardsArray[5];

            myBoard->getCards(tempBoardCardsArray);
        }
    }

    // unhighlight current players groupbox
    it_c = getActivePlayerIt(previousPlayerID);
    if (it_c != activePlayerList->end())
    {
        // lastPlayersTurn is active
        if (myEvents.onRefreshPlayersActiveInactiveStyles)
            myEvents.onRefreshPlayersActiveInactiveStyles(previousPlayerID, 1);
    }

    if (myEvents.onRefreshTableDescriptiveLabels)
        myEvents.onRefreshTableDescriptiveLabels(getCurrentRound());

    if (currentRound < GAME_STATE_POST_RIVER)
    {
        roundBeforePostRiver = currentRound;
    }

    switch (currentRound)
    {
    case GAME_STATE_PREFLOP:
    {
        if (myEvents.onPreflopAnimation)
            myEvents.onPreflopAnimation();
    }
    break;
    case GAME_STATE_FLOP:
    {
        if (myEvents.onFlopAnimation)
            myEvents.onFlopAnimation();
    }
    break;
    case GAME_STATE_TURN:
    {
        if (myEvents.onTurnAnimation)
            myEvents.onTurnAnimation();
    }
    break;
    case GAME_STATE_RIVER:
    {
        if (myEvents.onRiverAnimation)
            myEvents.onRiverAnimation();
    }
    break;
    case GAME_STATE_POST_RIVER:
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

    for (it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        if ((*it)->getID() == uniqueId)
        {
            break;
        }
    }

    return it;
}

PlayerListIterator Hand::getActivePlayerIt(unsigned uniqueId) const
{

    PlayerListIterator it;

    for (it = activePlayerList->begin(); it != activePlayerList->end(); ++it)
    {
        if ((*it)->getID() == uniqueId)
        {
            break;
        }
    }

    return it;
}

PlayerListIterator Hand::getRunningPlayerIt(unsigned uniqueId) const
{

    PlayerListIterator it;

    for (it = runningPlayerList->begin(); it != runningPlayerList->end(); ++it)
    {

        if ((*it)->getID() == uniqueId)
        {
            break;
        }
    }

    return it;
}

void Hand::setLastActionPlayerID(unsigned theValue)
{
    lastActionPlayerID = theValue;
    myBoard->setLastActionPlayerID(theValue);
}

int Hand::getPreflopCallsNumber()
{

    int calls = 0;

    for (PlayerListIterator it = activePlayerList->begin(); it != activePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getPreflopActions();

        if (find(actions.begin(), actions.end(), PLAYER_ACTION_CALL) != actions.end())
            calls++;
    }
    return calls;
}
int Hand::getPreflopRaisesNumber()
{

    int raises = 0;

    for (PlayerListIterator it = activePlayerList->begin(); it != activePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getPreflopActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PLAYER_ACTION_RAISE || (*itAction) == PLAYER_ACTION_ALLIN)
                raises++;
    }

    return raises;
}
int Hand::getFlopBetsOrRaisesNumber()
{

    int bets = 0;

    for (PlayerListIterator it = activePlayerList->begin(); it != activePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getFlopActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PLAYER_ACTION_RAISE || (*itAction) == PLAYER_ACTION_ALLIN ||
                (*itAction) == PLAYER_ACTION_BET)
                bets++;
    }

    return bets;
}
int Hand::getTurnBetsOrRaisesNumber()
{

    int bets = 0;

    for (PlayerListIterator it = activePlayerList->begin(); it != activePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getTurnActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PLAYER_ACTION_RAISE || (*itAction) == PLAYER_ACTION_ALLIN ||
                (*itAction) == PLAYER_ACTION_BET)
                bets++;
    }

    return bets;
}
int Hand::getRiverBetsOrRaisesNumber()
{

    int bets = 0;

    for (PlayerListIterator it = activePlayerList->begin(); it != activePlayerList->end(); ++it)
    {

        const std::vector<PlayerAction>& actions = (*it)->getCurrentHandActions().getRiverActions();

        for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
             itAction++)
            if ((*itAction) == PLAYER_ACTION_RAISE || (*itAction) == PLAYER_ACTION_ALLIN ||
                (*itAction) == PLAYER_ACTION_BET)
                bets++;
    }

    return bets;
}
std::vector<PlayerPosition> Hand::getRaisersPositions()
{

    std::vector<PlayerPosition> positions;

    PlayerListIterator it_c;

    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    { // note that all in players are not "running" any more

        if ((*it_c)->getAction() == PLAYER_ACTION_RAISE || (*it_c)->getAction() == PLAYER_ACTION_ALLIN)
            positions.push_back((*it_c)->getPosition());
    }
    return positions;
}

std::vector<PlayerPosition> Hand::getCallersPositions()
{

    std::vector<PlayerPosition> positions;

    PlayerListIterator it_c;

    for (it_c = runningPlayerList->begin(); it_c != runningPlayerList->end(); ++it_c)
    {

        if ((*it_c)->getAction() == PLAYER_ACTION_CALL)
            positions.push_back((*it_c)->getPosition());
    }
    return positions;
}
int Hand::getLastRaiserID()
{

    PlayerListIterator lastRaiser = activePlayerList->end();

    PlayerList players = activePlayerList;

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getAction() == PLAYER_ACTION_RAISE || (*it)->getAction() == PLAYER_ACTION_ALLIN)
        {

            if (lastRaiser != activePlayerList->end())
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
    if (lastRaiser != activePlayerList->end())
        return (*lastRaiser)->getID();

    // if no raiser was found, look for the one who have bet (if any)

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getAction() == PLAYER_ACTION_BET)
            lastRaiser = it;
    }
    if (lastRaiser != activePlayerList->end())
        return (*lastRaiser)->getID();
    else
        return -1;
}
int Hand::getPreflopLastRaiserID()
{
    return preflopLastRaiserID;
}

void Hand::setPreflopLastRaiserID(int id)
{
    preflopLastRaiserID = id;
}
int Hand::getFlopLastRaiserID()
{
    return flopLastRaiserID;
}

void Hand::setFlopLastRaiserID(int id)
{
    flopLastRaiserID = id;
}
int Hand::getTurnLastRaiserID()
{
    return turnLastRaiserID;
}

void Hand::setTurnLastRaiserID(int id)
{
    turnLastRaiserID = id;
}
} // namespace pkt::core
