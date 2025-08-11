#include "HandFsm.h"
#include <core/services/GlobalServices.h>
#include "CardUtilities.h"
#include "GameEvents.h"
#include "core/player/Helpers.h"
#include "model/ButtonState.h"

#include "Exception.h"
#include "model/EngineError.h"

#include "Helpers.h"
#include "core/engine/round_states/PreflopState.h"
#include "core/player/typedefs.h"

#include <random>

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

HandFsm::HandFsm(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
                 PlayerFsmList seats, PlayerFsmList runningPlayers, GameData gameData, StartData startData)
    : HandPlayersState(seats, runningPlayers), myEvents(events), myFactory(factory), myBoard(board),
      myStartQuantityPlayers(startData.numberOfPlayers), mySmallBlind(gameData.firstSmallBlind),
      myStartCash(gameData.startMoney)

{
    mySeatsList = seats;
    myRunningPlayersList = runningPlayers;
    myDealerPlayerId = startData.startDealerPlayerId;
    mySmallBlindPlayerId = startData.startDealerPlayerId;
    myBigBlindPlayerId = startData.startDealerPlayerId;
}

HandFsm::~HandFsm() = default;

void HandFsm::start()
{
    GlobalServices::instance().logger()->verbose(
        "\n----------------------  New hand initialization (FSM)  -------------------------------\n");

    initAndShuffleDeck();
    size_t cardsArrayIndex = dealBoardCards();
    dealHoleCards(cardsArrayIndex);

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->resetForNewHand();
    }

    getBettingState()->setPreflopLastRaiserId(-1);

    myState = std::make_unique<PreflopState>(myEvents, mySmallBlind, myDealerPlayerId);
    myState->enter(*this);
}

void HandFsm::end()
{
    myState.reset();
}

void HandFsm::handlePlayerAction(PlayerAction action)
{
    if (!myState)
        return;

    if (auto* processor = dynamic_cast<IActionProcessor*>(myState.get()))
    {
        if (!processor->isActionAllowed(*this, action))
            return;

        applyActionEffects(action);

        auto next = processor->computeNextState(*this, action);
        if (next)
        {
            myState->exit(*this);
            myState = std::move(next);
            myState->enter(*this);
        }
    }
}
void HandFsm::applyActionEffects(const PlayerAction& action)
{
    auto player = getPlayerFsmById(myRunningPlayersList, action.playerId);

    switch (action.type)
    {
    case ActionType::Fold:
        player->setActive(false);
        updateRunningPlayersListFsm(myRunningPlayersList);
        break;

    case ActionType::Call:
    {
        player->addBetAmount(action.amount);

        // myPot += toCall;
        break;
    }

    case ActionType::Raise:
    {
        /*int toCall = currentBet() - player.currentBet();
        int raiseAmount = action.amount();
        int totalPutIn = toCall + raiseAmount;

        player.stack -= totalPutIn;
        player.addToCurrentBet(totalPutIn);
        myPot += totalPutIn;
        myCurrentBet = player.currentBet(); // update table's highest bet
        myLastAggressor = player.id();
        */
        break;
    }
    case ActionType::Bet:
    {
        break;
    }
    case ActionType::Check:
    {
        break;
    }
    case ActionType::Allin:
    {
        break;
    }
    case ActionType::None:
    {
        break;
    }
    }

    // myActionHistory.push_back(action);
}

void HandFsm::initAndShuffleDeck()
{
    myCardsArray = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 rng(rd()); // Seed the Mersenne Twister random number generator
    std::shuffle(myCardsArray.begin(), myCardsArray.end(), rng);
}

void HandFsm::dealHoleCards(size_t cardsArrayIndex)
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
        GlobalServices::instance().logger()->verbose(
            "Player " + (*it)->getName() + " dealt cards: " + CardUtilities::getCardString(tempPlayerArray[0]) + " " +
            CardUtilities::getCardString(tempPlayerArray[1]) +
            ", hand strength = " + std::to_string((*it)->getHandRanking()));
    }
}

size_t HandFsm::dealBoardCards()
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

CommonHandContext HandFsm::updateCurrentHandContext(const GameState state)
{
    // general (and shared) game state
    CommonHandContext handContext;
    handContext.gameState = state;
    handContext.nbRunningPlayers = getRunningPlayersList()->size();
    handContext.lastVPIPPlayerFsm = getPlayerFsmById(getSeatsList(), getBettingState()->getLastRaiserId());
    handContext.callersPositions = myBettingState->getCallersPositions();
    handContext.raisersPositions = myBettingState->getRaisersPositions();
    handContext.pot = myBoard->getPot();
    // handContext.potOdd = getPotOdd();
    handContext.sets = myBoard->getSets();
    handContext.highestBetAmount = getBettingState()->getHighestSet();
    handContext.stringBoard = getStringBoard();
    handContext.preflopLastRaiserFsm = getPlayerFsmById(getSeatsList(), getBettingState()->getPreflopLastRaiserId());
    handContext.preflopRaisesNumber = getBettingState()->getPreflopRaisesNumber();
    handContext.preflopCallsNumber = getBettingState()->getPreflopCallsNumber();
    // handContext.isPreflopBigBet = getBettingState()->isPreflopBigBet();
    handContext.flopBetsOrRaisesNumber = getBettingState()->getFlopBetsOrRaisesNumber();
    handContext.flopLastRaiserFsm = getPlayerFsmById(getSeatsList(), getBettingState()->getFlopLastRaiserId());
    handContext.turnBetsOrRaisesNumber = getBettingState()->getTurnBetsOrRaisesNumber();
    handContext.turnLastRaiserFsm = getPlayerFsmById(getSeatsList(), getBettingState()->getTurnLastRaiserId());
    handContext.riverBetsOrRaisesNumber = getBettingState()->getRiverBetsOrRaisesNumber();
    handContext.nbPlayers = getSeatsList()->size();
    handContext.smallBlind = mySmallBlind;

    return handContext;
}
float HandFsm::getM(int cash) const
{
    int blinds = mySmallBlind + (mySmallBlind * 2);
    if (blinds > 0 && cash > 0)
    {
        return (float) cash / blinds;
    }
    else
    {
        return 0;
    }
}
std::string HandFsm::getStringBoard() const
{

    int cardsOnBoard;

    if (myState->getStateName() == "Flop")
    {
        cardsOnBoard = 3;
    }
    else if (myState->getStateName() == "Turn")
    {
        cardsOnBoard = 4;
    }
    else if (myState->getStateName() == "River")
    {
        cardsOnBoard = 5;
    }
    else
    {
        cardsOnBoard = 0;
    }

    std::string stringBoard;
    int board[5];
    myBoard->getCards(board);

    for (int i = 0; i < cardsOnBoard; i++)
    {
        stringBoard += " ";
        stringBoard += CardUtilities::getCardString(board[i]);
    }

    return stringBoard;
}

int HandFsm::getPotOdd(const int playerCash, const int playerSet) const
{
    const int highestBetAmount = min(playerCash, getBettingState()->getHighestSet());

    int pot = myBoard->getPot() + myBoard->getSets();

    if (pot == 0)
    { // shouldn't happen, but...
        GlobalServices::instance().logger()->error("Pot = " + std::to_string(myBoard->getPot()) + " + " +
                                                   std::to_string(myBoard->getSets()) + " = " + std::to_string(pot));
        return 0;
    }

    int odd = (highestBetAmount - playerSet) * 100 / pot;
    if (odd < 0)
    {
        odd = -odd; // happens if myTotalBetAmount > highestBetAmount
    }

    return odd;
}
PlayerPosition HandFsm::getPlayerPosition(const int playerId)
{

    PlayerPosition position = UNKNOWN;

    const int dealerPlayerId = myDealerPlayerId;
    const int nbPlayers = mySeatsList->size();

    // first dimension is my relative position, after the dealer.
    // Second dimension is the corrresponding position, depending on the number of players (from 0 to 10)
    const PlayerPosition onDealerPositionPlus[MAX_NUMBER_OF_PLAYERS][MAX_NUMBER_OF_PLAYERS + 1] = {
        // 0 player	1 player	2 players	3 players	4 players		5 players		6 players		7 players 8
        // players		9 players		10 players
        {UNKNOWN, UNKNOWN, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON,
         BUTTON},                                                                   // my position = dealer
        {UNKNOWN, UNKNOWN, BB, SB, SB, SB, SB, SB, SB, SB, SB},                     // my position = dealer + 1
        {UNKNOWN, UNKNOWN, UNKNOWN, BB, BB, BB, BB, BB, BB, BB, BB},                // my position = dealer + 2
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, UTG, UTG, UTG, UTG, UTG, UTG}, // my position = dealer + 3
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, MIDDLE, MIDDLE, UtgPlusOne, UtgPlusOne,
         UtgPlusOne}, // my position = dealer + 4
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE, MIDDLE, UtgPlusTwo,
         UtgPlusTwo}, // my position = dealer + 5
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE, MIDDLE,
         MIDDLE}, // my position = dealer + 6
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF, LATE,
         MiddlePlusOne}, // my position = dealer + 7
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, CUTOFF,
         LATE}, // my position = dealer + 8
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
         CUTOFF} // my position = dealer + 9
    };

    // first dimension is my relative position, BEHIND the dealer.
    // Second are the corrresponding positions, depending on the number of players
    const PlayerPosition onDealerPositionMinus[10][11] = {
        // 0 player	1 player	2 players	3 players	4 players		5 players		6 players		7 players 8
        // players		9 players		10 players
        {UNKNOWN, UNKNOWN, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON, BUTTON,
         BUTTON},                                                                           // my position = dealer
        {UNKNOWN, UNKNOWN, BB, BB, CUTOFF, CUTOFF, CUTOFF, CUTOFF, CUTOFF, CUTOFF, CUTOFF}, // my position = dealer - 1
        {UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, MIDDLE, LATE, LATE, LATE, LATE},           // my position = dealer - 2
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, MIDDLE, MIDDLE, MiddlePlusOne,
         MiddlePlusOne}, // my position = dealer - 3
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, UtgPlusOne, UtgPlusTwo,
         MIDDLE}, // my position = dealer - 4
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG, UtgPlusOne,
         UtgPlusTwo}, // my position = dealer - 5
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB, UTG,
         UtgPlusOne}, // my position = dealer - 6
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB, BB,
         UTG}, // my position = dealer - 7
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, SB,
         BB}, // my position = dealer - 8
        {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
         SB} // my position = dealer - 9

    };

    if (playerId == dealerPlayerId)
    {
        position = BUTTON;
    }
    else
    {

        // get my relative position from the dealer
        PlayerFsmListIterator itC;
        PlayerFsmList players = mySeatsList;

        int pos = 0;

        if (playerId > dealerPlayerId)
        {

            bool dealerFound = false;
            for (itC = players->begin(); itC != players->end(); ++itC)
            {
                if ((*itC)->getId() == dealerPlayerId)
                {
                    dealerFound = true;
                }
                else if (dealerFound)
                {
                    pos++;
                }
                if ((*itC)->getId() == playerId)
                {
                    break;
                }
            }
            position = onDealerPositionPlus[pos][nbPlayers];
        }
        else
        {
            // playerId < dealerPlayerId
            bool positionFound = false;
            for (itC = players->begin(); itC != players->end(); ++itC)
            {

                if ((*itC)->getId() == playerId)
                {
                    positionFound = true;
                }
                else if (positionFound)
                {
                    pos++;
                }
                if ((*itC)->getId() == dealerPlayerId)
                {
                    break;
                }
            }
            position = onDealerPositionMinus[pos][nbPlayers];
        }
    }

    assert(position != UNKNOWN);

    return position;
}

} // namespace pkt::core
