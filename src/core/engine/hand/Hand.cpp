#include "Hand.h"

#include "core/engine/EngineFactory.h"
#include "CardUtilities.h"
#include "DeckManager.h"
#include "GameEvents.h"
#include "HandCardDealer.h"
#include "HandCalculator.h"
#include "HandPlayersManager.h"
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

Hand::Hand(const GameEvents& events, std::shared_ptr<Board> board,
           PlayerList seats, PlayerList actingPlayers, GameData gameData, StartData startData,
           Logger& logger, PlayersStatisticsStore& statisticsStore,
           Randomizer& randomizer, HandEvaluationEngine& handEvaluationEngine)
    : m_events(events), m_board(board),
      m_logger(&logger), m_statisticsStore(&statisticsStore), m_randomizer(&randomizer),
      m_handEvaluationEngine(&handEvaluationEngine),
      m_actionValidator(std::make_unique<ActionValidator>()),
      m_startQuantityPlayers(startData.numberOfPlayers), m_smallBlind(gameData.firstSmallBlind), 
      m_startCash(gameData.startMoney)
{
    // Create HandPlayersManager to handle all player-related operations
    m_playersManager = std::make_unique<HandPlayersManager>(seats, actingPlayers, m_events, *m_logger);
    m_playersManager->setDealerPlayerId(startData.startDealerPlayerId);
    m_playersManager->setSmallBlindPlayerId(startData.startDealerPlayerId);
    m_playersManager->setBigBlindPlayerId(startData.startDealerPlayerId);

    // Create InvalidActionHandler with callbacks
    auto errorProvider = [this](const PlayerAction& action) -> std::string { return getActionValidationError(action); };
    auto autoFoldCallback = [this](unsigned playerId) { handleAutoFold(playerId); };
    m_invalidActionHandler = std::make_unique<InvalidActionHandler>(m_events, errorProvider, autoFoldCallback, *m_logger);

    // Create HandStateManager with error callback for game loop issues
    auto gameLoopErrorCallback = [this](const std::string& error)
    {
        if (m_events.onEngineError)
        {
            m_events.onEngineError(error);
        }
    };
    
    auto deckManager = std::make_unique<DeckManager>(*m_randomizer);
    m_cardDealer = std::make_unique<HandCardDealer>(std::move(deckManager), m_events, *m_logger, *m_handEvaluationEngine);
    m_calculator = std::make_unique<HandCalculator>(*m_logger);
    m_stateManager = std::make_unique<HandStateManager>(m_events, m_smallBlind, startData.startDealerPlayerId,
                                                        gameLoopErrorCallback, *m_logger);
}

Hand::~Hand() = default;


Logger& Hand::getLogger() const
{
    return *m_logger;
}

PlayersStatisticsStore& Hand::getPlayersStatisticsStore() const
{
    return *m_statisticsStore;
}

void Hand::initialize()
{
    getLogger().info("\n----------------------  New hand ----------------------------\n");

    initAndShuffleDeck();

    filterPlayersWithInsufficientCash();

    m_playersManager->preparePlayersForNewHand(*this);

    getBettingActions()->getPreflop().setLastRaiser(nullptr);

    m_stateManager->initializeState(*this);
}

void Hand::end()
{
    getPlayersStatisticsStore().savePlayersStatistics(getSeatsList());
}

void Hand::runGameLoop()
{
    dealHoleCards(0); // Pass 0 as index, since no board cards dealt yet

    m_stateManager->runGameLoop(*this);
}

void Hand::handlePlayerAction(PlayerAction action)
{
    auto* processor = getActionProcessor();

    if (!processor || !processor->isActionAllowed(*this, action))
    {
        m_invalidActionHandler->handleInvalidAction(action);
        return;
    }

    processValidAction(action);
}
void Hand::initAndShuffleDeck()
{
    m_cardDealer->initializeAndShuffleDeck();
}

void Hand::dealHoleCards(size_t cardsArrayIndex)
{
    m_cardDealer->dealHoleCards(getActingPlayersList(), *m_board);
}

size_t Hand::dealBoardCards()
{
    return m_cardDealer->dealBoardCards(*m_board);
}

std::vector<Card> Hand::dealCardsFromDeck(int numCards)
{
    return m_cardDealer->dealCardsFromDeck(numCards);
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
        
    handContext.playersContext.callersPositions = getBettingActions()->getCallersPositions();
    handContext.playersContext.raisersPositions = getBettingActions()->getRaisersPositions();
    
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
    return m_calculator->calculateM(cash, m_smallBlind);
}
std::string Hand::getStringBoard() const
{
    return m_board->getStringRepresentation();
}

int Hand::getPotOdd(const int playerCash, const int playerSet) const
{
    int pot = m_board->getPot(*this);
    int sets = m_board->getSets(*this);
    int roundHighestSet = getBettingActions()->getRoundHighestSet();
    
    return m_calculator->calculatePotOdd(playerCash, playerSet, pot, sets, roundHighestSet, getLogger());
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

    auto player = m_playersManager->validatePlayer(action.playerId);
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
        if (!m_actionValidator->validatePlayerActionWithReason(getActingPlayersList(), action, *getBettingActions(),
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
    getLogger().error("Player " + std::to_string(playerId) +
                               " exceeded maximum invalid actions, auto-folding");

    // If the game state is terminal, don't try to process any actions
    if (m_stateManager->isTerminal())
    {
        getLogger().error("Cannot auto-fold player " + std::to_string(playerId) + " - game state is terminal");

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

        ActionApplier::apply(*this, action, *m_logger);

        // Delegate state transition to HandStateManager
        m_stateManager->transitionToNextState(*this);
    }
    catch (const std::exception& e)
    {
        if (m_events.onEngineError)
        {
            m_events.onEngineError("Error processing player action: " + std::string(e.what()));
        }

        getLogger().error("Error in handlePlayerAction: " + std::string(e.what()));

        // Re-throw critical errors
        throw;
    }
}

// filterPlayersWithInsufficientCash() now handled by HandPlayersManager

} // namespace pkt::core
