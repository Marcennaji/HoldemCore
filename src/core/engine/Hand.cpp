#include "Hand.h"
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

#include <algorithm>
#include <random>

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

Hand::Hand(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
           PlayerList seats, PlayerList actingPlayers, GameData gameData, StartData startData)
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
        throw std::invalid_argument("Hand: smallBlind must be > 0");
    }
    if (myDealerPlayerId == static_cast<unsigned>(-1))
    {
        throw std::invalid_argument("Hand: dealerPlayerId is invalid");
    }
    if (startData.startDealerPlayerId >= static_cast<unsigned>(startData.numberOfPlayers))
    {
        throw std::invalid_argument("Hand: startDealerPlayerId is out of range");
    }
}

Hand::~Hand() = default;

void Hand::initialize()
{
    GlobalServices::instance().logger().info("\n----------------------  New hand ----------------------------\n");

    // Initialize deck but don't deal cards yet - wait until runGameLoop() to match legacy timing
    initAndShuffleDeck();

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        (*player)->resetForNewHand(*this);
    }

    getBettingActions()->getPreflop().setLastRaiserId(-1);

    myState = std::make_unique<PreflopState>(myEvents, mySmallBlind, myDealerPlayerId);
    myState->enter(*this);
}

void Hand::end()
{
    GlobalServices::instance().playersStatisticsStore().savePlayersStatistics(mySeatsList);
}

void Hand::runGameLoop()
{
    // Deal cards at the start of game loop to match legacy timing
    dealHoleCards(0); // Pass 0 as index, since no board cards dealt yet

    // Automatic game loop
    // This method drives the game forward by prompting players for actions

    int iterationCount = 0; // Emergency brake for infinite loops

    while (!myState->isTerminal() && iterationCount < MAX_GAME_LOOP_ITERATIONS)
    {
        iterationCount++;

        if (auto* processor = dynamic_cast<IActionProcessor*>(myState.get()))
        {
            auto nextPlayer = processor->getNextPlayerToAct(*this);
            if (nextPlayer)
            {
                processor->promptPlayerAction(*this, *nextPlayer);
            }
            else
            {
                // No next player to act, round should be complete
                // Transition to next state (e.g., flop -> turn)
                auto next = processor->computeNextState(*this);
                assert(next);
                myState->exit(*this);
                myState = std::move(next);
                myState->enter(*this);
            }
        }
    }

    // Check if we hit the emergency brake
    if (iterationCount >= MAX_GAME_LOOP_ITERATIONS)
    {
        GlobalServices::instance().logger().error("Game loop hit maximum iterations (" +
                                                  std::to_string(MAX_GAME_LOOP_ITERATIONS) +
                                                  "), terminating to prevent infinite loop");

        if (myEvents.onEngineError)
        {
            myEvents.onEngineError("Game loop terminated due to possible infinite loop");
        }

        // Force terminate the game
        return;
    }

    if (myState->isTerminal())
    {
        end();
    }
}

void Hand::handlePlayerAction(PlayerAction action)
{
    validateGameState();
    auto* processor = getActionProcessorOrThrow();

    if (!processor->isActionAllowed(*this, action))
    {
        handleInvalidAction(action);
        return;
    }

    processValidAction(action);
}
void Hand::applyActionEffects(const PlayerAction action)
{
    auto player = getPlayerById(myActingPlayersList, action.playerId);
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
        fireOnPotUpdated();
        break;
    }

    case ActionType::Raise:
    {
        int raiseIncrement = action.amount - playerBet;
        actionForHistory.amount = raiseIncrement; // store only the increment in history
        player->addBetAmount(raiseIncrement);
        fireOnPotUpdated();
        getBettingActions()->updateRoundHighestSet(action.amount);

        // Record last raiser for the current betting round
        switch (myState->getGameState())
        {
        case GameState::Preflop:
            getBettingActions()->getPreflop().setLastRaiserId(player->getId());
            break;
        case GameState::Flop:
            getBettingActions()->getFlop().setLastRaiserId(player->getId());
            break;
        case GameState::Turn:
            getBettingActions()->getTurn().setLastRaiserId(player->getId());
            break;
        case GameState::River:
            getBettingActions()->getRiver().setLastRaiserId(player->getId());
            break;
        default:
            break;
        }
        break;
    }

    case ActionType::Bet:
    {
        // For bet, the amount is already the increment
        player->addBetAmount(action.amount);
        fireOnPotUpdated();
        getBettingActions()->updateRoundHighestSet(action.amount);

        // Record last raiser (bettor) for the current betting round
        switch (myState->getGameState())
        {
        case GameState::Preflop:
            getBettingActions()->getPreflop().setLastRaiserId(player->getId());
            break;
        case GameState::Flop:
            getBettingActions()->getFlop().setLastRaiserId(player->getId());
            break;
        case GameState::Turn:
            getBettingActions()->getTurn().setLastRaiserId(player->getId());
            break;
        case GameState::River:
            getBettingActions()->getRiver().setLastRaiserId(player->getId());
            break;
        default:
            break;
        }
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
        fireOnPotUpdated();
        player->setCash(0);

        if (allinIncrement > currentHighest)
        {
            getBettingActions()->updateRoundHighestSet(allinIncrement);

            // Record last raiser (all-in as raise) for the current betting round
            switch (myState->getGameState())
            {
            case GameState::Preflop:
                getBettingActions()->getPreflop().setLastRaiserId(player->getId());
                break;
            case GameState::Flop:
                getBettingActions()->getFlop().setLastRaiserId(player->getId());
                break;
            case GameState::Turn:
                getBettingActions()->getTurn().setLastRaiserId(player->getId());
                break;
            case GameState::River:
                getBettingActions()->getRiver().setLastRaiserId(player->getId());
                break;
            default:
                break;
            }
        }
        break;
    }

    case ActionType::None:
    default:
        break;
    }

    player->setAction(*myState, actionForHistory);

    // Record action in hand-level chronological history
    getBettingActions()->recordPlayerAction(myState->getGameState(), actionForHistory);

    updateActingPlayersList(myActingPlayersList);

    if (myEvents.onPlayerActed)
    {
        myEvents.onPlayerActed(action);
    }
}

void Hand::initAndShuffleDeck()
{
    myDeck.initializeFullDeck();
    myDeck.shuffle();
}

void Hand::dealHoleCards(size_t cardsArrayIndex)
{
    // Validate that there are enough cards in the deck
    if (!myDeck.hasEnoughCards(0, mySeatsList->size())) // 0 board cards here since already dealt
    {
        throw std::runtime_error("Not enough cards in the deck to deal hole cards to all players.");
    }

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        // Deal 2 hole cards for this player
        std::vector<Card> holeCardList = myDeck.dealCards(2);

        // Create HoleCards from the dealt cards
        HoleCards holeCards(holeCardList[0], holeCardList[1]);

        (*it)->setHoleCards(holeCards);

        // Fire event for UI to know hole cards were dealt
        if (myEvents.onHoleCardsDealt)
        {
            myEvents.onHoleCardsDealt((*it)->getId(), holeCards);
        }

        // Create 7-card array for hand evaluation (2 hole + 5 board)
        int tempPlayerAndBoardArray[7];

        // Get board cards for evaluation
        BoardCards boardCards = myBoard->getBoardCards();
        for (int boardIndex = 0; boardIndex < 5; boardIndex++)
        {
            tempPlayerAndBoardArray[boardIndex] = boardCards.getCard(boardIndex).getIndex();
        }

        // Add hole cards
        tempPlayerAndBoardArray[5] = holeCardList[0].getIndex();
        tempPlayerAndBoardArray[6] = holeCardList[1].getIndex();

        // Evaluate hand using legacy string-based system
        string humanReadableHand = CardUtilities::getCardStringValue(tempPlayerAndBoardArray, 7);
        (*it)->setHandRanking(HandEvaluator::evaluateHand(humanReadableHand.c_str()));
    }
}

size_t Hand::dealBoardCards()
{
    // Deal 5 cards for the board (flop, turn, river)
    std::vector<Card> boardCardList = myDeck.dealCards(5);

    // Create BoardCards and progressively deal them
    BoardCards boardCards;

    // Deal flop first (3 cards)
    boardCards.dealFlop(boardCardList[0], boardCardList[1], boardCardList[2]);

    // Deal turn (4th card)
    boardCards.dealTurn(boardCardList[3]);

    // Deal river (5th card)
    boardCards.dealRiver(boardCardList[4]);

    myBoard->setBoardCards(boardCards);

    return 5; // Number of cards dealt
}

std::vector<Card> Hand::dealCardsFromDeck(int numCards)
{
    return myDeck.dealCards(numCards);
}

HandCommonContext Hand::updateHandCommonContext(const GameState state)
{
    // general (and shared) game state
    HandCommonContext handContext;
    handContext.gameState = state;
    handContext.stringBoard = getStringBoard();
    handContext.smallBlind = mySmallBlind;

    handContext.playersContext.actingPlayersList = getActingPlayersList();
    handContext.playersContext.lastVPIPPlayer = getPlayerById(getSeatsList(), getBettingActions()->getLastRaiserId());
    handContext.playersContext.callersPositions = myBettingActions->getCallersPositions();
    handContext.playersContext.raisersPositions = myBettingActions->getRaisersPositions();
    handContext.playersContext.preflopLastRaiser =
        getPlayerById(getSeatsList(), getBettingActions()->getPreflop().getLastRaiserId());
    handContext.playersContext.turnLastRaiser =
        getPlayerById(getSeatsList(), getBettingActions()->getTurn().getLastRaiserId());
    handContext.playersContext.flopLastRaiser =
        getPlayerById(getSeatsList(), getBettingActions()->getFlop().getLastRaiserId());

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
float Hand::getM(int cash) const
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
std::string Hand::getStringBoard() const
{
    const BoardCards& boardCards = myBoard->getBoardCards();

    // Use modern BoardCards toString but adjust for legacy format compatibility
    std::string boardString = boardCards.toString();

    if (boardString == "<no cards>" || boardString == "Invalid Board State")
    {
        return ""; // Legacy behavior for preflop/invalid states
    }

    // Add leading space to match legacy format (cards have spaces between them in toString())
    return " " + boardString;
}

int Hand::getPotOdd(const int playerCash, const int playerSet) const
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

IActionProcessor* Hand::getActionProcessor() const
{
    return dynamic_cast<IActionProcessor*>(myState.get());
}

int Hand::getSmallBlind() const
{
    return mySmallBlind;
}
void Hand::fireOnPotUpdated() const
{
    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(myBoard->getPot(*this));
    }
}

std::string Hand::getActionValidationError(const PlayerAction& action) const
{
    if (!myState)
    {
        return "Game state is invalid";
    }

    auto* processor = dynamic_cast<IActionProcessor*>(myState.get());
    if (!processor)
    {
        return "Current game state does not accept player actions";
    }

    auto player = getPlayerById(myActingPlayersList, action.playerId);
    if (!player)
    {
        return "Player not found in active players list";
    }

    // Check if it's the player's turn
    auto currentPlayer = processor->getNextPlayerToAct(*this);
    if (!currentPlayer || currentPlayer->getId() != action.playerId)
    {
        return "It's not this player's turn to act";
    }

    // Check specific action validation based on type
    switch (action.type)
    {
    case ActionType::Fold:
        return "Fold action is always valid"; // This shouldn't be called for valid folds

    case ActionType::Check:
        if (getBettingActions()->getRoundHighestSet() >
            player->getCurrentHandActions().getRoundTotalBetAmount(myState->getGameState()))
        {
            return "Cannot check when there is a bet to call";
        }
        break;

    case ActionType::Call:
        if (getBettingActions()->getRoundHighestSet() <=
            player->getCurrentHandActions().getRoundTotalBetAmount(myState->getGameState()))
        {
            return "Cannot call when no bet needs to be called";
        }
        break;

    case ActionType::Raise:
        if (action.amount <= getBettingActions()->getRoundHighestSet())
        {
            return "Raise amount must be higher than current highest bet";
        }
        if (action.amount >
            player->getCash() + player->getCurrentHandActions().getRoundTotalBetAmount(myState->getGameState()))
        {
            return "Raise amount exceeds available chips";
        }
        break;

    case ActionType::Bet:
        if (getBettingActions()->getRoundHighestSet() > 0)
        {
            return "Cannot bet when there is already a bet in this round";
        }
        if (action.amount > player->getCash())
        {
            return "Bet amount exceeds available chips";
        }
        break;

    case ActionType::Allin:
        // All-in is generally valid if player has chips
        if (player->getCash() <= 0)
        {
            return "Cannot go all-in with no chips";
        }
        break;

    default:
        return "Unknown or unsupported action type";
    }

    return "Action validation failed for unknown reason";
}

PlayerAction Hand::getDefaultActionForPlayer(unsigned playerId) const
{
    PlayerAction defaultAction;
    defaultAction.playerId = playerId;
    defaultAction.type = ActionType::Fold; // Default to fold for safety
    defaultAction.amount = 0;
    return defaultAction;
}

void Hand::resetInvalidActionCount(unsigned playerId)
{
    myInvalidActionCounts[playerId] = 0;
}

bool Hand::shouldAutoFoldPlayer(unsigned playerId) const
{
    auto it = myInvalidActionCounts.find(playerId);
    return (it != myInvalidActionCounts.end() && it->second >= MAX_INVALID_ACTIONS);
}

void Hand::validateGameState() const
{
    if (!myState)
    {
        throw Exception(__FILE__, __LINE__, EngineError::PlayerActionError);
    }
}

IActionProcessor* Hand::getActionProcessorOrThrow() const
{
    auto* processor = dynamic_cast<IActionProcessor*>(myState.get());
    if (!processor)
    {
        // Current state doesn't support actions - this is a programming error
        throw Exception(__FILE__, __LINE__, EngineError::PlayerActionError);
    }
    return processor;
}

void Hand::handleInvalidAction(const PlayerAction& action)
{
    // Track invalid action attempts
    myInvalidActionCounts[action.playerId]++;

    // Fire event to notify UI of invalid action
    if (myEvents.onInvalidPlayerAction)
    {
        std::string reason = getActionValidationError(action);
        myEvents.onInvalidPlayerAction(action.playerId, action, reason);
    }

    GlobalServices::instance().logger().error("Invalid action from player " + std::to_string(action.playerId) +
                                              " (attempt " + std::to_string(myInvalidActionCounts[action.playerId]) +
                                              "): " + getActionValidationError(action));

    // Check if player should be auto-folded due to repeated invalid actions
    if (shouldAutoFoldPlayer(action.playerId))
    {
        handleAutoFold(action.playerId);
    }
}

void Hand::handleAutoFold(unsigned playerId)
{
    GlobalServices::instance().logger().error("Player " + std::to_string(playerId) +
                                              " exceeded maximum invalid actions, auto-folding");

    // Create a fold action as default
    PlayerAction autoFoldAction = getDefaultActionForPlayer(playerId);

    if (myEvents.onEngineError)
    {
        myEvents.onEngineError("Player " + std::to_string(playerId) + " auto-folded due to repeated invalid actions");
    }

    // Reset counter and process the auto-fold
    resetInvalidActionCount(playerId);

    // Recursively call with the auto-fold action
    handlePlayerAction(autoFoldAction);
}

void Hand::processValidAction(const PlayerAction& action)
{
    try
    {
        // Reset invalid action count on successful action
        resetInvalidActionCount(action.playerId);

        applyActionEffects(action);

        auto* processor = getActionProcessorOrThrow();
        auto next = processor->computeNextState(*this);
        if (next)
        {
            myState->exit(*this);
            myState = std::move(next);
            myState->enter(*this);
            if (myState->isTerminal())
            {
                myState->exit(*this);
                end();
            }
        }
    }
    catch (const std::exception& e)
    {
        if (myEvents.onEngineError)
        {
            myEvents.onEngineError("Error processing player action: " + std::string(e.what()));
        }

        GlobalServices::instance().logger().error("Error in handlePlayerAction: " + std::string(e.what()));

        // Re-throw critical errors
        throw;
    }
}

} // namespace pkt::core
