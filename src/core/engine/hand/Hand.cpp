#include "Hand.h"

#include "CardUtilities.h"
#include "DeckManager.h"
#include "GameEvents.h"
#include "HandActionHandler.h"
#include "HandCalculator.h"
#include "HandCardDealer.h"
#include "HandPlayersManager.h"
#include "core/engine/EngineFactory.h"
#include "core/engine/actions/ActionApplier.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/Helpers.h"

#include "Helpers.h"
#include "core/engine/state/PreflopState.h"
#include "core/player/typedefs.h"

#include <algorithm>
#include <random>

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

Hand::Hand(const GameEvents& events, std::shared_ptr<Board> board, PlayerList seats, PlayerList actingPlayers,
           GameData gameData, StartData startData, Logger& logger, PlayersStatisticsStore& statisticsStore,
           Randomizer& randomizer, HandEvaluationEngine& handEvaluationEngine)
    : m_events(events), m_board(board), m_logger(&logger), m_statisticsStore(&statisticsStore),
      m_randomizer(&randomizer), m_handEvaluationEngine(&handEvaluationEngine),
      m_startQuantityPlayers(startData.numberOfPlayers), m_smallBlind(gameData.firstSmallBlind),
      m_startCash(gameData.startMoney)
{
    // Initialize all component managers using helper methods
    initializePlayersManager(seats, actingPlayers, startData);
    initializeActionHandler();
    initializeCardAndStateComponents(startData);
    initializeLifecycleManager();
}

Hand::~Hand() = default;

void Hand::initializePlayersManager(const pkt::core::player::PlayerList& seats,
                                    const pkt::core::player::PlayerList& actingPlayers, const StartData& startData)
{
    // Create HandPlayersManager to handle all player-related operations
    m_playersManager = std::make_unique<HandPlayersManager>(seats, actingPlayers, m_events, *m_logger);
    m_playersManager->setDealerPlayerId(startData.startDealerPlayerId);
    m_playersManager->setSmallBlindPlayerId(startData.startDealerPlayerId);
    m_playersManager->setBigBlindPlayerId(startData.startDealerPlayerId);
}

void Hand::initializeActionHandler()
{
    // Create ActionValidator and InvalidActionHandler with callbacks
    auto actionValidator = std::make_unique<ActionValidator>();

    // Create callbacks for InvalidActionHandler
    auto errorProvider = [this](const PlayerAction& action) -> std::string
    { return m_actionHandler->getActionValidationError(action, *this, *m_playersManager); };
    auto autoFoldCallback = [this](unsigned playerId) { m_actionHandler->handleAutoFold(playerId, *this); };
    auto invalidActionHandler =
        std::make_unique<InvalidActionHandler>(m_events, errorProvider, autoFoldCallback, *m_logger);

    // Create HandActionHandler to handle all action processing
    m_actionHandler = std::make_unique<HandActionHandler>(m_events, *m_logger, std::move(actionValidator),
                                                          std::move(invalidActionHandler));
}

void Hand::initializeCardAndStateComponents(const StartData& startData)
{
    // Create HandStateManager with error callback for game loop issues
    auto gameLoopErrorCallback = [this](const std::string& error)
    {
        if (m_events.onEngineError)
        {
            m_events.onEngineError(error);
        }
    };

    // Create card dealing and calculation components
    auto deckManager = std::make_unique<DeckManager>(*m_randomizer);
    m_cardDealer =
        std::make_unique<HandCardDealer>(std::move(deckManager), m_events, *m_logger, *m_handEvaluationEngine);
    m_calculator = std::make_unique<HandCalculator>(*m_logger);
    m_stateManager = std::make_unique<HandStateManager>(m_events, m_smallBlind, startData.startDealerPlayerId,
                                                        gameLoopErrorCallback, *m_logger);
}

void Hand::initializeLifecycleManager()
{
    // Create HandLifecycleManager to handle lifecycle operations
    m_lifecycleManager = std::make_unique<HandLifecycleManager>(*m_logger, *m_statisticsStore);
}

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
    m_lifecycleManager->initialize(*this);
}

void Hand::end()
{
    m_lifecycleManager->end(*this);
}

void Hand::runGameLoop()
{
    m_lifecycleManager->runGameLoop(*this);
}

void Hand::handlePlayerAction(PlayerAction action)
{
    m_actionHandler->handlePlayerAction(action, *this);
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
    HandCommonContext handContext;

    populateGeneralGameContext(handContext);
    populatePlayersContextInfo(handContext);
    populateBettingContextInfo(handContext);

    return handContext;
}

void Hand::populateGeneralGameContext(HandCommonContext& context)
{
    // General (and shared) game state
    context.gameState = m_stateManager->getGameState();
    context.stringBoard = getStringBoard();
    context.smallBlind = m_smallBlind;
}

void Hand::populatePlayersContextInfo(HandCommonContext& context)
{
    context.playersContext.actingPlayersList = getActingPlayersList();
    context.playersContext.nbPlayers = getSeatsList()->size();

    // Get last VPIP player - keep existing logic for now since BettingActions::getLastRaiserId() is more complex
    int lastVPIPPlayerId = getBettingActions()->getLastRaiserId();
    context.playersContext.lastVPIPPlayer =
        (lastVPIPPlayerId != -1) ? getPlayerById(getSeatsList(), lastVPIPPlayerId) : nullptr;

    context.playersContext.callersPositions = getBettingActions()->getCallersPositions();
    context.playersContext.raisersPositions = getBettingActions()->getRaisersPositions();

    // Directly assign last raiser pointers - they're already validated by the BettingRoundActions
    context.playersContext.preflopLastRaiser = getBettingActions()->getPreflop().getLastRaiser();
    context.playersContext.flopLastRaiser = getBettingActions()->getFlop().getLastRaiser();
    context.playersContext.turnLastRaiser = getBettingActions()->getTurn().getLastRaiser();
}

void Hand::populateBettingContextInfo(HandCommonContext& context)
{
    context.bettingContext.pot = m_board->getPot(*this);
    // context.bettingContext.potOdd = getPotOdd();
    context.bettingContext.sets = m_board->getSets(*this);
    context.bettingContext.highestBetAmount = getBettingActions()->getRoundHighestSet();
    context.bettingContext.preflopRaisesNumber = getBettingActions()->getPreflop().getRaisesNumber();
    context.bettingContext.preflopCallsNumber = getBettingActions()->getPreflop().getCallsNumber();
    // context.bettingContext.isPreflopBigBet = getBettingActions()->isPreflopBigBet();
    context.bettingContext.flopBetsOrRaisesNumber = getBettingActions()->getFlop().getBetsOrRaisesNumber();
    context.bettingContext.turnBetsOrRaisesNumber = getBettingActions()->getTurn().getBetsOrRaisesNumber();
    context.bettingContext.riverBetsOrRaisesNumber = getBettingActions()->getRiver().getBetsOrRaisesNumber();
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

} // namespace pkt::core
