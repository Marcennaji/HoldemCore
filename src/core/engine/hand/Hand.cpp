#include "Hand.h"
#include <core/services/ServiceContainer.h>
#include "core/engine/EngineFactory.h"
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

Hand::Hand(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<Board> board,
           PlayerList seats, PlayerList actingPlayers, GameData gameData, StartData startData)
    : HandPlayersState(seats, actingPlayers), m_events(events), m_factory(factory), m_board(board), m_services(nullptr),
      m_deckManager(std::make_unique<DeckManager>()), m_actionValidator(std::make_unique<ActionValidator>()),
      m_startQuantityPlayers(startData.numberOfPlayers), m_smallBlind(gameData.firstSmallBlind),
      m_startCash(gameData.startMoney)
{
    m_seatsList = seats;
    m_actingPlayersList = actingPlayers;
    m_dealerPlayerId = startData.startDealerPlayerId;
    m_smallBlindPlayerId = startData.startDealerPlayerId;
    m_bigBlindPlayerId = startData.startDealerPlayerId;

    // Create InvalidActionHandler with callbacks
    auto errorProvider = [this](const PlayerAction& action) -> std::string { return getActionValidationError(action); };

    auto autoFoldCallback = [this](unsigned playerId) { handleAutoFold(playerId); };

    m_invalidActionHandler = std::make_unique<InvalidActionHandler>(m_events, errorProvider, autoFoldCallback);

    // Create HandStateManager with error callback for game loop issues
    auto gameLoopErrorCallback = [this](const std::string& error)
    {
        if (m_events.onEngineError)
        {
            m_events.onEngineError(error);
        }
    };
    
    // Pass services to HandStateManager for proper dependency injection
    ensureServicesInitialized();
    m_stateManager = std::make_unique<HandStateManager>(m_events, m_smallBlind, startData.startDealerPlayerId,
                                                        gameLoopErrorCallback, m_services);
}

Hand::Hand(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<Board> board,
           PlayerList seats, PlayerList actingPlayers, GameData gameData, StartData startData,
           std::shared_ptr<PokerServices> services)
    : HandPlayersState(seats, actingPlayers), m_events(events), m_factory(factory), m_board(board),
      m_services(services), m_deckManager(std::make_unique<DeckManager>()),
      m_actionValidator(std::make_unique<ActionValidator>()), m_startQuantityPlayers(startData.numberOfPlayers),
      m_smallBlind(gameData.firstSmallBlind), m_startCash(gameData.startMoney)
{
    m_seatsList = seats;
    m_actingPlayersList = actingPlayers;
    m_dealerPlayerId = startData.startDealerPlayerId;
    m_smallBlindPlayerId = startData.startDealerPlayerId;
    m_bigBlindPlayerId = startData.startDealerPlayerId;

    // Create InvalidActionHandler with callbacks
    auto errorProvider = [this](const PlayerAction& action) -> std::string { return getActionValidationError(action); };

    auto autoFoldCallback = [this](unsigned playerId) { handleAutoFold(playerId); };

    m_invalidActionHandler = std::make_unique<InvalidActionHandler>(m_events, errorProvider, autoFoldCallback);

    // Create HandStateManager with error callback for game loop issues
    auto gameLoopErrorCallback = [this](const std::string& error)
    {
        if (m_events.onEngineError)
        {
            m_events.onEngineError(error);
        }
    };
    
    // Pass services to HandStateManager for proper dependency injection
    ensureServicesInitialized();
    m_stateManager = std::make_unique<HandStateManager>(m_events, m_smallBlind, startData.startDealerPlayerId,
                                                        gameLoopErrorCallback, m_services);
}

Hand::~Hand() = default;

void Hand::ensureServicesInitialized() const
{
    if (!m_services)
    {
        // Get services from the factory to ensure ISP compliance and test service injection
        auto factoryServices = m_factory->getServiceContainer();
        m_services = std::make_shared<PokerServices>(factoryServices);
    }
}

void Hand::initialize()
{
    ensureServicesInitialized();
    m_services->logger().info("\n----------------------  New hand ----------------------------\n");

    // Initialize deck but don't deal cards yet - wait until runGameLoop() to match legacy timing
    initAndShuffleDeck();

    // Remove players with insufficient cash before dealing cards
    filterPlayersWithInsufficientCash();

    for (auto player = m_seatsList->begin(); player != m_seatsList->end(); ++player)
    {
        (*player)->resetForNewHand(*this);
    }

    getBettingActions()->getPreflop().setLastRaiser(nullptr);

    m_stateManager->initializeState(*this);
}

void Hand::end()
{
    ensureServicesInitialized();
    m_services->playersStatisticsStore().savePlayersStatistics(m_seatsList);
}

void Hand::runGameLoop()
{
    // Deal cards at the start of game loop to match legacy timing
    dealHoleCards(0); // Pass 0 as index, since no board cards dealt yet

    // Delegate game loop management to HandStateManager
    m_stateManager->runGameLoop(*this);
}

void Hand::handlePlayerAction(PlayerAction action)
{
    auto* processor = getActionProcessor();

    // If there's no processor, it means the game state doesn't accept actions
    if (!processor || !processor->isActionAllowed(*this, action))
    {
        m_invalidActionHandler->handleInvalidAction(action);
        return;
    }

    processValidAction(action);
}
void Hand::initAndShuffleDeck()
{
    m_deckManager->initializeAndShuffle();
}

void Hand::dealHoleCards(size_t cardsArrayIndex)
{
    // Validate that there are enough cards in the deck
    if (!m_deckManager->hasEnoughCards(0, m_actingPlayersList->size())) // Use acting players list size
    {
        throw std::runtime_error("Not enough cards in the deck to deal hole cards to all players.");
    }

    for (auto it = m_actingPlayersList->begin(); it != m_actingPlayersList->end(); ++it)
    {
        // Deal 2 hole cards for this player
        std::vector<Card> holeCardList = m_deckManager->dealCards(2);

        // Create HoleCards from the dealt cards
        HoleCards holeCards(holeCardList[0], holeCardList[1]);

        (*it)->setHoleCards(holeCards);

        // Fire event for UI to know hole cards were dealt
        if (m_events.onHoleCardsDealt)
        {
            m_events.onHoleCardsDealt((*it)->getId(), holeCards);
        }

        // Build evaluator string with correct ordering: HOLE cards first, then current BOARD cards.
        // Avoid including invalid board placeholders at preflop to prevent identical ranks.
        BoardCards boardCards = m_board->getBoardCards();
        std::string humanReadableHand = holeCardList[0].toString() + std::string(" ") + holeCardList[1].toString();
        if (boardCards.getNumCards() > 0)
        {
            humanReadableHand += std::string(" ") + boardCards.toString();
        }
        // Prefer using existing services to avoid creating a new container inside HandEvaluator
        (*it)->setHandRanking(HandEvaluator::evaluateHand(humanReadableHand.c_str(), m_services));
    }
}

size_t Hand::dealBoardCards()
{
    // Deal 5 cards for the board (flop, turn, river)
    std::vector<Card> boardCardList = m_deckManager->dealCards(5);

    // Create BoardCards and progressively deal them
    BoardCards boardCards;

    // Deal flop first (3 cards)
    boardCards.dealFlop(boardCardList[0], boardCardList[1], boardCardList[2]);

    // Deal turn (4th card)
    boardCards.dealTurn(boardCardList[3]);

    // Deal river (5th card)
    boardCards.dealRiver(boardCardList[4]);

    m_board->setBoardCards(boardCards);

    return 5; // Number of cards dealt
}

std::vector<Card> Hand::dealCardsFromDeck(int numCards)
{
    return m_deckManager->dealCards(numCards);
}

HandCommonContext Hand::updateHandCommonContext()
{
    // general (and shared) game state
    HandCommonContext handContext;
    handContext.gameState = m_stateManager->getGameState();
    handContext.stringBoard = getStringBoard();
    handContext.smallBlind = m_smallBlind;

    handContext.playersContext.actingPlayersList = getActingPlayersList();
    
    // Get last VPIP player - keep existing logic for now since BettingActions::getLastRaiserId() is more complex
    int lastVPIPPlayerId = getBettingActions()->getLastRaiserId();
    handContext.playersContext.lastVPIPPlayer = 
        (lastVPIPPlayerId != -1) ? getPlayerById(getSeatsList(), lastVPIPPlayerId) : nullptr;
        
    handContext.playersContext.callersPositions = m_bettingActions->getCallersPositions();
    handContext.playersContext.raisersPositions = m_bettingActions->getRaisersPositions();
    
    // Directly assign last raiser pointers - they're already validated by the BettingRoundActions
    handContext.playersContext.preflopLastRaiser = getBettingActions()->getPreflop().getLastRaiser();
    handContext.playersContext.flopLastRaiser = getBettingActions()->getFlop().getLastRaiser();
    handContext.playersContext.turnLastRaiser = getBettingActions()->getTurn().getLastRaiser();

    handContext.bettingContext.pot = m_board->getPot(*this);
    // handContext.bettingContext.potOdd = getPotOdd();
    handContext.bettingContext.sets = m_board->getSets(*this);
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
    int blinds = m_smallBlind + (m_smallBlind * 2);
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
    const BoardCards& boardCards = m_board->getBoardCards();

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

    int pot = m_board->getPot(*this) + m_board->getSets(*this);

    if (pot == 0)
    { // shouldn't happen, but...
        ensureServicesInitialized();
        m_services->logger().error("Pot = " + std::to_string(m_board->getPot(*this)) + " + " +
                                   std::to_string(m_board->getSets(*this)) + " = " + std::to_string(pot));
        return 0;
    }

    int odd = (highestBetAmount - playerSet) * 100 / pot;
    if (odd < 0)
    {
        odd = -odd; // happens if m_totalBetAmount > highestBetAmount
    }

    return odd;
}

HandActionProcessor* Hand::getActionProcessor() const
{
    return m_stateManager->getActionProcessor();
}

int Hand::getSmallBlind() const
{
    return m_smallBlind;
}
void Hand::fireOnPotUpdated() const
{
    if (m_events.onPotUpdated)
    {
        m_events.onPotUpdated(m_board->getPot(*this));
    }
}

std::string Hand::getActionValidationError(const PlayerAction& action) const
{
    if (m_stateManager->isTerminal())
    {
        return "Game state is terminal";
    }

    auto* processor = m_stateManager->getActionProcessor();
    if (!processor)
    {
        return "Current game state does not accept player actions";
    }

    auto player = getPlayerById(m_actingPlayersList, action.playerId);
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

    // Use the comprehensive ActionValidator with a detailed reason
    {
        std::string reason;
        if (!m_actionValidator->validatePlayerActionWithReason(m_actingPlayersList, action, *getBettingActions(),
                                                               m_smallBlind, m_stateManager->getGameState(), reason))
        {
            return reason.empty() ? std::string("Action validation failed.") : reason;
        }
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
    m_services->logger().error("Player " + std::to_string(playerId) +
                               " exceeded maximum invalid actions, auto-folding");

    // If the game state is terminal, don't try to process any actions
    if (m_stateManager->isTerminal())
    {
        m_services->logger().error("Cannot auto-fold player " + std::to_string(playerId) + " - game state is terminal");

        if (m_events.onEngineError)
        {
            m_events.onEngineError("Player " + std::to_string(playerId) +
                                   " attempted action in terminal state - no auto-fold processed");
        }
        return;
    }

    // Create a fold action as default
    PlayerAction autoFoldAction = getDefaultActionForPlayer(playerId);

    if (m_events.onEngineError)
    {
        m_events.onEngineError("Player " + std::to_string(playerId) + " auto-folded due to repeated invalid actions");
    }

    // Recursively call with the auto-fold action
    handlePlayerAction(autoFoldAction);
}

void Hand::processValidAction(const PlayerAction& action)
{
    try
    {
        // Reset invalid action count on successful action
        m_invalidActionHandler->resetInvalidActionCount(action.playerId);

        ActionApplier::apply(*this, action);

        // Delegate state transition to HandStateManager
        m_stateManager->transitionToNextState(*this);
    }
    catch (const std::exception& e)
    {
        if (m_events.onEngineError)
        {
            m_events.onEngineError("Error processing player action: " + std::string(e.what()));
        }

        ensureServicesInitialized();
        m_services->logger().error("Error in handlePlayerAction: " + std::string(e.what()));

        // Re-throw critical errors
        throw;
    }
}

void Hand::filterPlayersWithInsufficientCash()
{
    ensureServicesInitialized();
    
    // Remove players from acting players list who cannot afford minimum participation
    // Cards will only be dealt to players in the acting list, so this is sufficient
    auto it = m_actingPlayersList->begin();
    while (it != m_actingPlayersList->end())
    {
        const int playerCash = (*it)->getCash();
        
        // Players with zero cash cannot participate at all
        if (playerCash <= 0)
        {
            m_services->logger().info("Player " + (*it)->getName() + " (ID: " + 
                std::to_string((*it)->getId()) + ") auto-folded due to insufficient cash: " + 
                std::to_string(playerCash));
            
            // Create a fold action to notify the UI that this player is folded
            PlayerAction autoFoldAction;
            autoFoldAction.playerId = (*it)->getId();
            autoFoldAction.type = ActionType::Fold;
            autoFoldAction.amount = 0;
            
            // Fire the player action event to notify UI that this player folded
            if (m_events.onPlayerActed)
            {
                m_events.onPlayerActed(autoFoldAction);
            }
                
            it = m_actingPlayersList->erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    // Log the number of players remaining for the hand
    m_services->logger().debug("Hand will proceed with " + std::to_string(m_actingPlayersList->size()) + " players");
}

} // namespace pkt::core
