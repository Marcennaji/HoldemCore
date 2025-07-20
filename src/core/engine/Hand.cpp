// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Hand.h"
#include <core/services/GlobalServices.h>
#include "CardUtilities.h"
#include "GameEvents.h"
#include "Randomizer.h"
#include "core/player/Helpers.h"
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
           PlayerList seats, PlayerList runningPlayers, GameData gameData, StartData startData)
    : myEvents(events), myFactory(factory), myBoard(board), mySeatsList(seats), myRunningPlayersList(runningPlayers),
      myStartQuantityPlayers(startData.numberOfPlayers), myDealerPlayerId(startData.startDealerPlayerId),
      mySmallBlindPlayerId(startData.startDealerPlayerId), myBigBlindPlayerId(startData.startDealerPlayerId),
      mySmallBlind(gameData.firstSmallBlind), myStartCash(gameData.startMoney)

{

    GlobalServices::instance().logger()->verbose(
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
        GlobalServices::instance().logger()->verbose(
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
        (*it)->setButton(ButtonNone);
    }

    // assign dealer button
    it = getPlayerListIteratorById(mySeatsList, myDealerPlayerId);
    if (it == mySeatsList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
    }
    (*it)->setButton(ButtonDealer);

    // assign Small Blind next to dealer. ATTENTION: in heads up it is big blind
    // assign big blind next to small blind. ATTENTION: in heads up it is small blind
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
                (*it)->setButton(2);
                mySmallBlindPlayerId = (*it)->getId();
            }
            else
            {
                // big blind in heads up
                (*it)->setButton(3);
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
                (*it)->setButton(3);
                myBigBlindPlayerId = (*it)->getId();
            }
            else
            {
                // small blind in heads up
                (*it)->setButton(2);
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

    PlayerListConstIterator itC;

    // do sets --> TODO switch?
    for (itC = myRunningPlayersList->begin(); itC != myRunningPlayersList->end(); ++itC)
    {

        // small blind
        if ((*itC)->getButton() == ButtonSmallBlind)
        {

            // All in ?
            if ((*itC)->getCash() <= mySmallBlind)
            {

                (*itC)->setSet((*itC)->getCash());
                // 1 to do not log this
                (*itC)->setAction(ActionType::Allin, 1);
            }
            else
            {
                (*itC)->setSet(mySmallBlind);
            }
        }
    }

    // do sets --> TODO switch?
    for (itC = myRunningPlayersList->begin(); itC != myRunningPlayersList->end(); ++itC)
    {

        // big blind
        if ((*itC)->getButton() == ButtonBigBlind)
        {

            // all in ?
            if ((*itC)->getCash() <= 2 * mySmallBlind)
            {

                (*itC)->setSet((*itC)->getCash());
                // 1 to do not log this
                (*itC)->setAction(ActionType::Allin, 1);
            }
            else
            {
                (*itC)->setSet(2 * mySmallBlind);
            }
        }
    }
}

void Hand::updateRunningPlayersList()
{
    GlobalServices::instance().logger()->verbose("Updating myRunningPlayersList...");

    PlayerListIterator it, it1;

    for (it = myRunningPlayersList->begin(); it != myRunningPlayersList->end();)
    {
        GlobalServices::instance().logger()->verbose("Checking player: " + (*it)->getName() +
                                                     ", action: " + playerActionToString((*it)->getAction()));

        if ((*it)->getAction() == ActionType::Fold || (*it)->getAction() == ActionType::Allin)
        {
            GlobalServices::instance().logger()->verbose(
                "Removing player: " + (*it)->getName() +
                " from myRunningPlayersList due to action: " + playerActionToString((*it)->getAction()));

            it = myRunningPlayersList->erase(it);

            if (!myRunningPlayersList->empty())
            {
                GlobalServices::instance().logger()->verbose(
                    "myRunningPlayersList is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myRunningPlayersList->begin())
                {
                    GlobalServices::instance().logger()->verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myRunningPlayersList->end();
                }
                --it1;

                GlobalServices::instance().logger()->verbose("Setting current player's turn to: " + (*it1)->getName() +
                                                             " (ID: " + std::to_string((*it1)->getId()) + ")");
                getCurrentBettingRound()->setCurrentPlayerTurnId((*it1)->getId());
            }
            else
            {
                GlobalServices::instance().logger()->verbose("myRunningPlayersList is now empty after removal.");
            }
        }
        else
        {
            GlobalServices::instance().logger()->verbose(
                "Player: " + (*it)->getName() + " remains in myRunningPlayersList. Moving to the next player.");
            ++it;
        }
    }

    GlobalServices::instance().logger()->verbose("Finished updating myRunningPlayersList.");
}

void Hand::resolveHandConditions()
{
    GlobalServices::instance().logger()->verbose("Executing resolveHandConditions() for betting round: " +
                                                 std::to_string(myCurrentRound));

    PlayerListIterator it, it1;
    PlayerListConstIterator itC;

    // Log the current state of the running player list
    GlobalServices::instance().logger()->verbose("Current running players:");
    for (auto& player : *myRunningPlayersList)
    {
        GlobalServices::instance().logger()->verbose("Player " + player->getName() +
                                                     " action: " + playerActionToString(player->getAction()) +
                                                     ", set: " + std::to_string(player->getSet()));
    }

    updateRunningPlayersList();

    // Determine number of all-in players
    int allInPlayersCounter = 0;
    for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    {
        if ((*itC)->getAction() == ActionType::Allin)
        {
            allInPlayersCounter++;
        }
    }
    GlobalServices::instance().logger()->verbose("Number of all-in players: " + std::to_string(allInPlayersCounter));

    // Determine number of non-fold players
    int nonFoldPlayerCounter = 0;
    for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    {
        if ((*itC)->getAction() != ActionType::Fold)
        {
            nonFoldPlayerCounter++;
        }
    }
    GlobalServices::instance().logger()->verbose("Number of non-fold players: " + std::to_string(nonFoldPlayerCounter));

    // If only one player non-fold -> distribute pot
    if (nonFoldPlayerCounter == 1)
    {
        GlobalServices::instance().logger()->verbose("Only one non-fold player remains. Distributing pot.");
        myBoard->collectPot();
        if (myEvents.onPotUpdated)
        {
            myEvents.onPotUpdated(myBoard->getPot());
        }

        myCurrentRound = GameStatePostRiver;
        GlobalServices::instance().logger()->verbose("Set current round to GameStatePostRiver.");
    }
    else
    {
        // Check for all-in condition
        if (allInPlayersCounter == nonFoldPlayerCounter)
        {
            GlobalServices::instance().logger()->verbose("All players are all-in.");
            myAllInCondition = true;
            myBoard->setAllInCondition(true);
        }
        else if (allInPlayersCounter + 1 == nonFoldPlayerCounter)
        {
            GlobalServices::instance().logger()->verbose("All players but one are all-in.");
            for (itC = myRunningPlayersList->begin(); itC != myRunningPlayersList->end(); ++itC)
            {
                if ((*itC)->getSet() >= myBettingRounds[myCurrentRound]->getHighestSet())
                {
                    myAllInCondition = true;
                    myBoard->setAllInCondition(true);
                    GlobalServices::instance().logger()->verbose(
                        "Player " + (*itC)->getName() + " has the highest set and triggered all-in condition.");
                }
            }
        }
    }

    // Special routine for all-in condition
    if (myAllInCondition)
    {
        GlobalServices::instance().logger()->verbose("Handling all-in condition.");
        myBoard->collectPot();

        if (myEvents.onPotUpdated)
        {
            myEvents.onPotUpdated(myBoard->getPot());
        }

        if (myEvents.onFlipHoleCardsAllIn)
        {
            myEvents.onFlipHoleCardsAllIn();
        }

        if (myCurrentRound < GameStatePostRiver)
        {
            myCurrentRound = GameState(myCurrentRound + 1);
            GlobalServices::instance().logger()->verbose("Advanced to next round: " + std::to_string(myCurrentRound));
        }

        if (myCurrentRound >= GameStateFlop)
        {
            int tempBoardCardsArray[5];
            myBoard->getCards(tempBoardCardsArray);
            GlobalServices::instance().logger()->verbose("Board cards logged for all-in condition.");
        }
    }
    itC = getPlayerListIteratorById(mySeatsList, myPreviousPlayerId);
    if (itC != mySeatsList->end())
    {
        if (myEvents.onPlayerStatusChanged)
        {
            myEvents.onPlayerStatusChanged(myPreviousPlayerId, false);
            GlobalServices::instance().logger()->verbose("Unhighlighted previous player's groupbox: " +
                                                         std::to_string(myPreviousPlayerId));
        }
    }

    if (myCurrentRound < GameStatePostRiver)
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

    for (PlayerListIterator it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getPreflopActions();

        if (find(actions.begin(), actions.end(), ActionType::Call) != actions.end())
        {
            calls++;
        }
    }
    return calls;
}
int Hand::getPreflopRaisesNumber()
{

    int raises = 0;

    for (PlayerListIterator it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getPreflopActions();

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin)
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

    for (PlayerListIterator it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getFlopActions();

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin || (*itAction) == ActionType::Bet)
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

    for (PlayerListIterator it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getTurnActions();

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin || (*itAction) == ActionType::Bet)
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

    for (PlayerListIterator it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {

        const std::vector<ActionType>& actions = (*it)->getCurrentHandActions().getRiverActions();

        for (std::vector<ActionType>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
        {
            if ((*itAction) == ActionType::Raise || (*itAction) == ActionType::Allin || (*itAction) == ActionType::Bet)
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

        if ((*itC)->getAction() == ActionType::Raise || (*itC)->getAction() == ActionType::Allin)
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

    for (itC = myRunningPlayersList->begin(); itC != myRunningPlayersList->end(); ++itC)
    {

        if ((*itC)->getAction() == ActionType::Call)
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

        if ((*it)->getAction() == ActionType::Raise || (*it)->getAction() == ActionType::Allin)
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

        if ((*it)->getAction() == ActionType::Bet)
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

std::shared_ptr<IBettingRoundStateFsm> Hand::getCurrentRoundStateFsm() const
{
    return myCurrentStateFsm;
}
void Hand::setCurrentRoundStateFsm(std::shared_ptr<IBettingRoundStateFsm> b)
{
    myCurrentStateFsm = b;
}

void Hand::applyActionFsm(const pkt::core::PlayerAction&)
{
}
void Hand::advanceToNextPlayerFsm()
{
}
bool Hand::isBettingRoundCompleteFsm() const
{
    return false;
}
bool Hand::canAcceptActionFsm(PlayerAction) const
{
    return false;
}
void Hand::postBlindsFsm()
{
}
void Hand::prepareBettingRoundFsm()
{
}
void Hand::dealFlopFsm()
{
}
void Hand::dealTurnFsm()
{
}
void Hand::dealRiverFsm()
{
}

void Hand::handlePlayerActionFsm(PlayerAction action)
{
    assert(myCurrentStateFsm && "Betting state must be initialized");

    auto player = getPlayerById(getSeatsList(), action.playerId);

    // 1. Ask current FSM state if the action is allowed
    if (!myCurrentStateFsm->canProcessAction(*this, action))
    {
        throw std::logic_error("Action not allowed in current state");
    }

    // 2. Process the action via the FSM
    std::unique_ptr<IBettingRoundStateFsm> nextState = myCurrentStateFsm->processAction(*this, action);

    // 3. Apply the state transition, if any
    if (nextState)
    {
        myCurrentStateFsm->exit(*this);
        myCurrentStateFsm = std::move(nextState);
        myCurrentStateFsm->enter(*this);
    }

    // 4. Notify event listeners (e.g., UI)
    if (myEvents.onPlayerActed)
    {
        myEvents.onPlayerActed(action);
    }

    // 5. Optionally log state
    myCurrentStateFsm->logStateInfo(*this);
}
} // namespace pkt::core
