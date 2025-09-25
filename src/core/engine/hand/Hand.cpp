#include "Hand.h"
#include <core/services/ServiceContainer.h>
#include "CardUtilities.h"
#include "DeckManager.h"
#include "GameEvents.h"
#include "core/engine/actions/ActionApplier.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"

#include "Exception.h"
#include "model/EngineError.h"

#include "Helpers.h"
#include "core/engine/state/PreflopState.h"
#include "core/player/typedefs.h"

#include <algorithm>
#include <random>

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

Hand::Hand(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
           PlayerList seats, PlayerList actingPlayers, GameData gameData, StartData startData)
    : HandPlayersState(seats, actingPlayers), myEvents(events), myFactory(factory), myBoard(board), myServices(nullptr),
      myDeckManager(std::make_unique<DeckManager>()), myActionValidator(std::make_unique<ActionValidator>()),
      myStartQuantityPlayers(startData.numberOfPlayers), mySmallBlind(gameData.firstSmallBlind),
      myStartCash(gameData.startMoney)
{
    mySeatsList = seats;
    myActingPlayersList = actingPlayers;
    myDealerPlayerId = startData.startDealerPlayerId;
    mySmallBlindPlayerId = startData.startDealerPlayerId;
    myBigBlindPlayerId = startData.startDealerPlayerId;

    // Create InvalidActionHandler with callbacks
    auto errorProvider = [this](const PlayerAction& action) -> std::string { return getActionValidationError(action); };
}

Hand::Hand(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
           PlayerList seats, PlayerList actingPlayers, GameData gameData, StartData startData,
           std::shared_ptr<PokerServices> services)
    : HandPlayersState(seats, actingPlayers), myEvents(events), myFactory(factory), myBoard(board),
      myServices(services), myDeckManager(std::make_unique<DeckManager>()),
      myActionValidator(std::make_unique<ActionValidator>()), myStartQuantityPlayers(startData.numberOfPlayers),
      mySmallBlind(gameData.firstSmallBlind), myStartCash(gameData.startMoney)
{
    mySeatsList = seats;
    myActingPlayersList = actingPlayers;
    myDealerPlayerId = startData.startDealerPlayerId;
    mySmallBlindPlayerId = startData.startDealerPlayerId;
    myBigBlindPlayerId = startData.startDealerPlayerId;

    // Create InvalidActionHandler with callbacks
    auto errorProvider = [this](const PlayerAction& action) -> std::string { return getActionValidationError(action); };

    auto autoFoldCallback = [this](unsigned playerId) { handleAutoFold(playerId); };

    myInvalidActionHandler = std::make_unique<InvalidActionHandler>(myEvents, errorProvider, autoFoldCallback);

    // Create HandStateManager with error callback for game loop issues
    auto gameLoopErrorCallback = [this](const std::string& error)
    {
        if (myEvents.onEngineError)
        {
            myEvents.onEngineError(error);
        }
    };
    myStateManager = std::make_unique<HandStateManager>(myEvents, mySmallBlind, startData.startDealerPlayerId,
                                                        gameLoopErrorCallback);

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

void Hand::ensureServicesInitialized() const
{
    if (!myServices)
    {
        auto baseContainer = std::make_shared<AppServiceContainer>();
        myServices = std::make_shared<PokerServices>(baseContainer);
    }
}

void Hand::initialize()
{
    ensureServicesInitialized();
    myServices->logger().info("\n----------------------  New hand ----------------------------\n");

    // Initialize deck but don't deal cards yet - wait until runGameLoop() to match legacy timing
    initAndShuffleDeck();

    for (auto player = mySeatsList->begin(); player != mySeatsList->end(); ++player)
    {
        (*player)->resetForNewHand(*this);
    }

    getBettingActions()->getPreflop().setLastRaiserId(-1);

    myStateManager->initializeState(*this);
}

void Hand::end()
{
    ensureServicesInitialized();
    myServices->playersStatisticsStore().savePlayersStatistics(mySeatsList);
}

void Hand::runGameLoop()
{
    // Deal cards at the start of game loop to match legacy timing
    dealHoleCards(0); // Pass 0 as index, since no board cards dealt yet

    // Delegate game loop management to HandStateManager
    myStateManager->runGameLoop(*this);
}

void Hand::handlePlayerAction(PlayerAction action)
{
    auto* processor = getActionProcessor();

    // If there's no processor, it means the game state doesn't accept actions
    if (!processor || !processor->isActionAllowed(*this, action))
    {
        myInvalidActionHandler->handleInvalidAction(action);
        return;
    }

    processValidAction(action);
}
void Hand::initAndShuffleDeck()
{
    myDeckManager->initializeAndShuffle();
}

void Hand::dealHoleCards(size_t cardsArrayIndex)
{
    // Validate that there are enough cards in the deck
    if (!myDeckManager->hasEnoughCards(0, mySeatsList->size())) // 0 board cards here since already dealt
    {
        throw std::runtime_error("Not enough cards in the deck to deal hole cards to all players.");
    }

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        // Deal 2 hole cards for this player
        std::vector<Card> holeCardList = myDeckManager->dealCards(2);

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
    // Prefer using existing services to avoid creating a new container inside HandEvaluator
    (*it)->setHandRanking(HandEvaluator::evaluateHand(humanReadableHand.c_str(), myServices));
    }
}

size_t Hand::dealBoardCards()
{
    // Deal 5 cards for the board (flop, turn, river)
    std::vector<Card> boardCardList = myDeckManager->dealCards(5);

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
    return myDeckManager->dealCards(numCards);
}

HandCommonContext Hand::updateHandCommonContext()
{
    // general (and shared) game state
    HandCommonContext handContext;
    handContext.gameState = myStateManager->getGameState();
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
        ensureServicesInitialized();
        myServices->logger().error("Pot = " + std::to_string(myBoard->getPot(*this)) + " + " +
                                   std::to_string(myBoard->getSets(*this)) + " = " + std::to_string(pot));
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
    return myStateManager->getActionProcessor();
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
    if (myStateManager->isTerminal())
    {
        return "Game state is terminal";
    }

    auto* processor = myStateManager->getActionProcessor();
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

    // Use the comprehensive ActionValidator
    if (!myActionValidator->validatePlayerAction(myActingPlayersList, action, *getBettingActions(), mySmallBlind,
                                                 myStateManager->getGameState()))
    {
        return "Action validation failed.";
    }

    return ""; // Empty string means action is valid
}

PlayerAction Hand::getDefaultActionForPlayer(unsigned playerId) const
{
    PlayerAction defaultAction;
    defaultAction.playerId = playerId;
    defaultAction.type = ActionType::Fold; // Default to fold for safety
    defaultAction.amount = 0;
    return defaultAction;
}

void Hand::handleAutoFold(unsigned playerId)
{
    ensureServicesInitialized();
    myServices->logger().error("Player " + std::to_string(playerId) +
                               " exceeded maximum invalid actions, auto-folding");

    // If the game state is terminal, don't try to process any actions
    if (myStateManager->isTerminal())
    {
        myServices->logger().error("Cannot auto-fold player " + std::to_string(playerId) + " - game state is terminal");

        if (myEvents.onEngineError)
        {
            myEvents.onEngineError("Player " + std::to_string(playerId) +
                                   " attempted action in terminal state - no auto-fold processed");
        }
        return;
    }

    // Create a fold action as default
    PlayerAction autoFoldAction = getDefaultActionForPlayer(playerId);

    if (myEvents.onEngineError)
    {
        myEvents.onEngineError("Player " + std::to_string(playerId) + " auto-folded due to repeated invalid actions");
    }

    // Recursively call with the auto-fold action
    handlePlayerAction(autoFoldAction);
}

void Hand::processValidAction(const PlayerAction& action)
{
    try
    {
        // Reset invalid action count on successful action
        myInvalidActionHandler->resetInvalidActionCount(action.playerId);

        ActionApplier::apply(*this, action);

        // Delegate state transition to HandStateManager
        myStateManager->transitionToNextState(*this);
    }
    catch (const std::exception& e)
    {
        if (myEvents.onEngineError)
        {
            myEvents.onEngineError("Error processing player action: " + std::string(e.what()));
        }

        ensureServicesInitialized();
        myServices->logger().error("Error in handlePlayerAction: " + std::string(e.what()));

        // Re-throw critical errors
        throw;
    }
}

} // namespace pkt::core
