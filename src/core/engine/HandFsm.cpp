#include "HandFsm.h"
#include <core/services/GlobalServices.h>
#include "CardUtilities.h"
#include "GameEvents.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"

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
                 PlayerFsmList seats, PlayerFsmList actingPlayers, GameData gameData, StartData startData)
    : HandPlayersState(seats, actingPlayers), myEvents(events), myFactory(factory), myBoard(board),
      myStartQuantityPlayers(startData.numberOfPlayers), mySmallBlind(gameData.firstSmallBlind),
      myStartCash(gameData.startMoney)

{
    mySeatsList = seats;
    myActingPlayersList = actingPlayers;
    myDealerPlayerId = startData.startDealerPlayerId;
    mySmallBlindPlayerId = startData.startDealerPlayerId;
    myBigBlindPlayerId = startData.startDealerPlayerId;

    if (mySmallBlind <= 0)
    {
        throw std::invalid_argument("HandFsm: smallBlind must be > 0");
    }
    if (myDealerPlayerId == static_cast<unsigned>(-1))
    {
        throw std::invalid_argument("HandFsm: dealerPlayerId is invalid");
    }
    if (startData.startDealerPlayerId >= static_cast<unsigned>(startData.numberOfPlayers))
    {
        throw std::invalid_argument("HandFsm: startDealerPlayerId is out of range");
    }
}

HandFsm::~HandFsm() = default;

void HandFsm::start()
{
    GlobalServices::instance().logger().info(
        "\n----------------------  New hand initialization (FSM)  -------------------------------\n");

    initAndShuffleDeck();
    size_t cardsArrayIndex = dealBoardCards();
    dealHoleCards(cardsArrayIndex);

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        (*player)->resetForNewHand(*this);
    }

    getBettingActions()->getPreflop().setLastRaiserId(-1);

    myState = std::make_unique<PreflopState>(myEvents, mySmallBlind, myDealerPlayerId);
    myState->enter(*this);
}

void HandFsm::end()
{
    GlobalServices::instance().playersStatisticsStore().savePlayersStatistics(mySeatsList);
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
            if (myState->isTerminal())
                end();
        }
    }
}
void HandFsm::applyActionEffects(const PlayerAction action)
{
    auto player = getPlayerFsmById(myActingPlayersList, action.playerId);
    if (!player)
        return;

    int currentHighest = getBettingActions()->getRoundHighestSet();
    int playerBet = player->getCurrentHandActions().getRoundTotalBetAmount(myState->getGameState());

    // Create a copy for storing in action history with correct increment amounts
    PlayerAction actionForHistory = action;

    switch (action.type)
    {
    case ActionType::Fold:
        // No amount change needed
        break;

    case ActionType::Call:
    {
        int amountToCall = currentHighest - playerBet;
        if (player->getCash() < amountToCall)
        {
            amountToCall = player->getCash();
        }

        // If this call uses all remaining cash, treat it as an all-in
        if (amountToCall == player->getCash())
        {
            actionForHistory.type = ActionType::Allin;
            player->setCash(0);
        }

        actionForHistory.amount = amountToCall; // store the actual call amount
        player->addBetAmount(amountToCall);
        break;
    }

    case ActionType::Raise:
    {
        int raiseIncrement = action.amount - playerBet;
        actionForHistory.amount = raiseIncrement; // store only the increment in history
        player->addBetAmount(raiseIncrement);
        getBettingActions()->updateRoundHighestSet(action.amount);
        getBettingActions()->getPreflop().setLastRaiserId(player->getId());
        break;
    }

    case ActionType::Bet:
    {
        // For bet, the amount is already the increment
        player->addBetAmount(action.amount);
        getBettingActions()->updateRoundHighestSet(action.amount);
        break;
    }

    case ActionType::Check:
    {
        // No amount change needed
        break;
    }

    case ActionType::Allin:
    {
        int allinIncrement = player->getCash();   // The increment is all remaining cash
        actionForHistory.amount = allinIncrement; // store only the increment in history

        player->addBetAmount(allinIncrement);
        player->setCash(0);

        if (allinIncrement > currentHighest)
        {
            getBettingActions()->updateRoundHighestSet(allinIncrement);
            getBettingActions()->getPreflop().setLastRaiserId(player->getId());
        }
        break;
    }

    case ActionType::None:
    default:
        break;
    }

    player->setAction(*myState, actionForHistory);
    updateActingPlayersListFsm(myActingPlayersList);
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

HandCommonContext HandFsm::updateHandCommonContext(const GameState state)
{
    // general (and shared) game state
    HandCommonContext handContext;
    handContext.gameState = state;
    handContext.stringBoard = getStringBoard();
    handContext.smallBlind = mySmallBlind;

    handContext.playersContext.actingPlayersListFsm = getActingPlayersList();
    handContext.playersContext.lastVPIPPlayerFsm =
        getPlayerFsmById(getSeatsList(), getBettingActions()->getLastRaiserId());
    handContext.playersContext.callersPositions = myBettingActions->getCallersPositions();
    handContext.playersContext.raisersPositions = myBettingActions->getRaisersPositions();
    handContext.playersContext.preflopLastRaiserFsm =
        getPlayerFsmById(getSeatsList(), getBettingActions()->getPreflop().getLastRaiserId());
    handContext.playersContext.turnLastRaiserFsm =
        getPlayerFsmById(getSeatsList(), getBettingActions()->getTurn().getLastRaiserId());
    handContext.playersContext.flopLastRaiserFsm =
        getPlayerFsmById(getSeatsList(), getBettingActions()->getFlop().getLastRaiserId());

    handContext.bettingContext.pot = myBoard->getPot(*this);
    // handContext.bettingContext.potOdd = getPotOdd();
    handContext.bettingContext.sets = myBoard->getSets(*this);
    handContext.bettingContext.highestBetAmount = getBettingActions()->getRoundHighestSet();
    handContext.bettingContext.preflopRaisesNumber = getBettingActions()->getPreflop().getRaisesNumber();
    handContext.bettingContext.preflopCallsNumber = getBettingActions()->getPreflop().getCallsNumber();
    // handContext.bettingContext.isPreflopBigBet = getBettingActions()->isPreflopBigBet();
    handContext.bettingContext.flopBetsOrRaisesNumber = getBettingActions()->getFlop().getBetsOrRaisesNumber();
    handContext.bettingContext.turnBetsOrRaisesNumber = getBettingActions()->getTurn().getBetsOrRaisesNumber();
    handContext.bettingContext.riverBetsOrRaisesNumber = getBettingActions()->getRiver().getBetsOrRaisesNumber();
    handContext.playersContext.nbPlayers = getSeatsList()->size();

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

    if (myState->getGameState() == GameState::Flop)
    {
        cardsOnBoard = 3;
    }
    else if (myState->getGameState() == GameState::Turn)
    {
        cardsOnBoard = 4;
    }
    else if (myState->getGameState() == GameState::River)
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
    const int highestBetAmount = min(playerCash, getBettingActions()->getRoundHighestSet());

    int pot = myBoard->getPot(*this) + myBoard->getSets(*this);

    if (pot == 0)
    { // shouldn't happen, but...
        GlobalServices::instance().logger().error("Pot = " + std::to_string(myBoard->getPot(*this)) + " + " +
                                                  std::to_string(myBoard->getSets(*this)) + " = " +
                                                  std::to_string(pot));
        return 0;
    }

    int odd = (highestBetAmount - playerSet) * 100 / pot;
    if (odd < 0)
    {
        odd = -odd; // happens if myTotalBetAmount > highestBetAmount
    }

    return odd;
}

IActionProcessor* HandFsm::getActionProcessor() const
{
    return dynamic_cast<IActionProcessor*>(myState.get());
}

int HandFsm::getSmallBlind() const
{
    return mySmallBlind;
}

} // namespace pkt::core
