// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "core/engine/deprecated/Hand.h"
#include <core/services/GlobalServices.h>
#include "CardUtilities.h"
#include "GameEvents.h"
#include "core/engine/deprecated/ButtonState.h"
#include "core/player/Helpers.h"

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
           PlayerList seats, PlayerList actingPlayers, GameData gameData, StartData startData)
    : myEvents(events), myFactory(factory), myBoard(board), mySeatsList(seats), myActingPlayersList(actingPlayers),
      myStartQuantityPlayers(startData.numberOfPlayers), myDealerPlayerId(startData.startDealerPlayerId),
      mySmallBlindPlayerId(startData.startDealerPlayerId), myBigBlindPlayerId(startData.startDealerPlayerId),
      mySmallBlind(gameData.firstSmallBlind), myStartCash(gameData.startMoney)

{

    GlobalServices::instance().logger().verbose(
        "\n----------------------  New hand initialization  -------------------------------\n");

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

    myBettingRounds = myFactory->createAllBettingRounds(this, myDealerPlayerId, mySmallBlind);
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
    if (myCardsArray.size() < 5 + 2 * mySeatsList->size())
    {
        throw std::runtime_error("Not enough cards in the deck to deal hole cards and board cards.");
    }

    // Initialize the first 5 cards of the board
    for (boardCardIndex = 0; boardCardIndex < 5; boardCardIndex++)
    {
        tempPlayerAndBoardArray[boardCardIndex] = myCardsArray[boardCardIndex];
    }

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it, playerIndex++)
    {
        for (holeCardIndex = 0; holeCardIndex < 2; holeCardIndex++)
        {
            tempPlayerArray[holeCardIndex] = myCardsArray[2 * playerIndex + holeCardIndex + 5];
            tempPlayerAndBoardArray[5 + holeCardIndex] = myCardsArray[2 * playerIndex + holeCardIndex + 5];
        }
        string humanReadableHand = CardUtilities::getCardStringValue(tempPlayerAndBoardArray, 7);
        (*it)->setCards(tempPlayerArray);
        (*it)->setHandRanking(HandEvaluator::evaluateHand(humanReadableHand.c_str()));
        (*it)->setCashAtHandStart((*it)->getCash());
        (*it)->getCurrentHandActions().reset();
        (*it)->setPosition();
        (*it)->getRangeEstimator()->setEstimatedRange("");
        GlobalServices::instance().logger().verbose(
            "Player " + (*it)->getName() + " dealt cards: " + CardUtilities::getCardString(tempPlayerArray[0]) + " " +
            CardUtilities::getCardString(tempPlayerArray[1]) +
            ", hand strength = " + std::to_string((*it)->getHandRanking()));
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
    if (myEvents.onBettingRoundStarted)
    {
        myEvents.onBettingRoundStarted(getCurrentBettingRound()->getBettingRoundId());
    }
    if (myEvents.onDealHoleCards)
    {
        myEvents.onDealHoleCards();
    }

    getBoard()->collectSets();

    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(getBoard()->getPot());
    }

    resolveHandConditions();
}

void Hand::assignButtons()
{

    size_t i;
    PlayerListIterator it;
    PlayerListConstIterator itC;

    // delete all buttons
    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->setButton(ButtonState::Unspecified);
    }

    // assign dealer button
    it = getPlayerListIteratorById(mySeatsList, myDealerPlayerId);
    if (it == mySeatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }
    (*it)->setButton(Dealer);

    // NB. in heads-up mode, the Button is also Small Blind
    bool nextActivePlayerFound = false;
    auto dealerPositionIt = getPlayerListIteratorById(mySeatsList, myDealerPlayerId);
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
        it = getPlayerListIteratorById(mySeatsList, (*dealerPositionIt)->getId());
        if (it != mySeatsList->end())
        {
            nextActivePlayerFound = true;
            if (mySeatsList->size() > 2)
            {
                // small blind normal
                (*it)->setButton(SB);
                mySmallBlindPlayerId = (*it)->getId();
            }
            else
            {
                // big blind in heads up
                (*it)->setButton(BB);
                myBigBlindPlayerId = (*it)->getId();
                // lastPlayerAction for showing cards
            }

            // first player after dealer have to show his cards first (in showdown)
            myLastActionPlayerId = (*it)->getId();
            myBoard->setLastActionPlayerId(myLastActionPlayerId);

            ++it;
            if (it == mySeatsList->end())
            {
                it = mySeatsList->begin();
            }

            if (mySeatsList->size() > 2)
            {
                // big blind normal
                (*it)->setButton(BB);
                myBigBlindPlayerId = (*it)->getId();
            }
            else
            {
                // small blind in heads up
                (*it)->setButton(SB);
                mySmallBlindPlayerId = (*it)->getId();
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

    // do sets --> TODO switch?
    for (auto player = myActingPlayersList->begin(); player != myActingPlayersList->end(); ++player)
    {

        // small blind
        if ((*player)->getButton() == ButtonState::SB)
        {

            // All in ?
            if ((*player)->getCash() <= mySmallBlind)
            {

                (*player)->addBetAmount((*player)->getCash());
                (*player)->setLastAction({(*player)->getId(), ActionType::Allin, (*player)->getTotalBetAmount()}, 1);
            }
            else
            {
                (*player)->addBetAmount(mySmallBlind);
            }
        }
    }

    // do sets --> TODO switch?
    for (auto player = myActingPlayersList->begin(); player != myActingPlayersList->end(); ++player)
    {

        // big blind
        if ((*player)->getButton() == ButtonState::BB)
        {

            // all in ?
            if ((*player)->getCash() <= 2 * mySmallBlind)
            {

                (*player)->addBetAmount((*player)->getCash());

                (*player)->setLastAction({(*player)->getId(), ActionType::Allin, (*player)->getTotalBetAmount()}, 1);
            }
            else
            {
                (*player)->addBetAmount(2 * mySmallBlind);
            }
        }
    }
}

void Hand::resolveHandConditions()
{
    GlobalServices::instance().logger().verbose("Executing resolveHandConditions() for betting round: " +
                                                std::to_string(myCurrentRound));

    PlayerListIterator it, it1;
    PlayerListConstIterator itC;

    // Log the current state of the acting player list
    GlobalServices::instance().logger().verbose("Current acting players:");
    for (auto& player : *myActingPlayersList)
    {
        GlobalServices::instance().logger().verbose("Player " + player->getName() +
                                                    " action: " + playerActionToString(player->getLastAction().type) +
                                                    ", set: " + std::to_string(player->getTotalBetAmount()));
    }

    updateActingPlayersList(myActingPlayersList);

    // Determine number of all-in players
    int allInPlayersCounter = 0;
    for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    {
        if ((*itC)->getLastAction().type == ActionType::Allin)
        {
            allInPlayersCounter++;
        }
    }
    GlobalServices::instance().logger().verbose("Number of all-in players: " + std::to_string(allInPlayersCounter));

    // Determine number of non-fold players
    int nonFoldPlayerCounter = 0;
    for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    {
        if ((*itC)->getLastAction().type != ActionType::Fold)
        {
            nonFoldPlayerCounter++;
        }
    }
    GlobalServices::instance().logger().verbose("Number of non-fold players: " + std::to_string(nonFoldPlayerCounter));

    // If only one player non-fold -> distribute pot
    if (nonFoldPlayerCounter == 1)
    {
        GlobalServices::instance().logger().verbose("Only one non-fold player remains. Distributing pot.");
        myBoard->collectPot();
        if (myEvents.onPotUpdated)
        {
            myEvents.onPotUpdated(myBoard->getPot());
        }

        myCurrentRound = PostRiver;
        GlobalServices::instance().logger().verbose("Set current round to PostRiver.");
    }
    else
    {
        // Check for all-in condition
        if (allInPlayersCounter == nonFoldPlayerCounter)
        {
            GlobalServices::instance().logger().verbose("All players are all-in.");
            myAllInCondition = true;
            myBoard->setAllInCondition(true);
        }
        else if (allInPlayersCounter + 1 == nonFoldPlayerCounter)
        {
            GlobalServices::instance().logger().verbose("All players but one are all-in.");
            for (itC = myActingPlayersList->begin(); itC != myActingPlayersList->end(); ++itC)
            {
                if ((*itC)->getTotalBetAmount() >= myBettingRounds[myCurrentRound]->getRoundHighestSet())
                {
                    myAllInCondition = true;
                    myBoard->setAllInCondition(true);
                    GlobalServices::instance().logger().verbose("Player " + (*itC)->getName() +
                                                                " has the highest set and triggered all-in condition.");
                }
            }
        }
    }

    // Special routine for all-in condition
    if (myAllInCondition)
    {
        GlobalServices::instance().logger().verbose("Handling all-in condition.");
        myBoard->collectPot();

        if (myEvents.onPotUpdated)
        {
            myEvents.onPotUpdated(myBoard->getPot());
        }

        if (myEvents.onFlipHoleCardsAllIn)
        {
            myEvents.onFlipHoleCardsAllIn();
        }

        if (myCurrentRound < PostRiver)
        {
            myCurrentRound = GameState(myCurrentRound + 1);
            GlobalServices::instance().logger().verbose("Advanced to next round: " + std::to_string(myCurrentRound));
        }

        if (myCurrentRound >= Flop)
        {
            int tempBoardCardsArray[5];
            myBoard->getCards(tempBoardCardsArray);
            GlobalServices::instance().logger().verbose("Board cards logged for all-in condition.");
        }
    }
    itC = getPlayerListIteratorById(mySeatsList, myPreviousPlayerId);
    if (itC != mySeatsList->end())
    {
        if (myEvents.onPlayerStatusChanged)
        {
            myEvents.onPlayerStatusChanged(myPreviousPlayerId, false);
            GlobalServices::instance().logger().verbose("Unhighlighted previous player's groupbox: " +
                                                        std::to_string(myPreviousPlayerId));
        }
    }

    if (myCurrentRound < PostRiver)
    {
        myRoundBeforePostRiver = myCurrentRound;
    }

    getCurrentBettingRound()->run();
}

void Hand::setLastActionPlayerId(unsigned theValue)
{
    myLastActionPlayerId = theValue;
    myBoard->setLastActionPlayerId(theValue);
}

int Hand::getPreflopCallsNumber()
{

    int calls = 0;

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        auto& actions = (*player)->getCurrentHandActions().getActions(GameState::Preflop);

        for (const auto& action : actions)
        {
            if (action.type == ActionType::Call)
            {
                calls++;
            }
        }
    }
    return calls;
}
int Hand::getPreflopRaisesNumber()
{

    int raises = 0;

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        auto& actions = (*player)->getCurrentHandActions().getActions(GameState::Preflop);

        for (const auto& action : actions)
        {
            if (action.type == ActionType::Raise || action.type == ActionType::Allin)
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

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        auto& actions = (*player)->getCurrentHandActions().getActions(GameState::Flop);

        for (const auto& action : actions)
        {
            if (action.type == ActionType::Raise || action.type == ActionType::Allin || action.type == ActionType::Bet)
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

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        auto& actions = (*player)->getCurrentHandActions().getActions(GameState::Turn);

        for (const auto& action : actions)
        {
            if (action.type == ActionType::Bet || action.type == ActionType::Raise || action.type == ActionType::Allin)
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

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        auto& actions = (*player)->getCurrentHandActions().getActions(GameState::River);

        for (const auto& action : actions)
        {
            if (action.type == ActionType::Bet || action.type == ActionType::Raise || action.type == ActionType::Allin)
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

    for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    { // note that all in players are not "running" any more

        if ((*itC)->getLastAction().type == ActionType::Raise || (*itC)->getLastAction().type == ActionType::Allin)
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

    for (itC = myActingPlayersList->begin(); itC != myActingPlayersList->end(); ++itC)
    {

        if ((*itC)->getLastAction().type == ActionType::Call)
        {
            positions.push_back((*itC)->getPosition());
        }
    }
    return positions;
}
int Hand::getLastRaiserId()
{

    PlayerListIterator lastRaiser = mySeatsList->end();

    PlayerList players = mySeatsList;

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getLastAction().type == ActionType::Raise || (*it)->getLastAction().type == ActionType::Allin)
        {

            if (lastRaiser != mySeatsList->end())
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
    if (lastRaiser != mySeatsList->end())
    {
        return (*lastRaiser)->getId();
    }

    // if no raiser was found, look for the one who have bet (if any)

    for (PlayerListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getLastAction().type == ActionType::Bet)
        {
            lastRaiser = it;
        }
    }
    if (lastRaiser != mySeatsList->end())
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
