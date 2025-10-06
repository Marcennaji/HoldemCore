#include "FlopState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "TurnState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/utils/Helpers.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace pkt::core::player;

FlopState::FlopState(const GameEvents& events) : m_events(events)
{
}

// ISP-compliant constructor - only accepts what it actually needs
FlopState::FlopState(const GameEvents& events, std::shared_ptr<Logger> logger) 
    : m_events(events), m_loggerService(logger)
{
}

void FlopState::enter(Hand& hand)
{
    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    if (m_events.onBettingRoundStarted)
        m_events.onBettingRoundStarted(Flop);

    // Deal flop cards (3 cards) and fire event with flop-specific BoardCards
    BoardCards currentBoard = hand.getBoard().getBoardCards();
    if (currentBoard.getNumCards() == 0) // Only deal if not already dealt
    {
        // Get next 3 cards from deck for flop
        std::vector<Card> flopCards = hand.dealCardsFromDeck(3);

        // Create flop-specific BoardCards (3 cards only)
        BoardCards flopBoard(flopCards[0], flopCards[1], flopCards[2]);

        // Update board with flop cards
        hand.getBoard().setBoardCards(flopBoard);

        // Fire event with flop-specific board (3 cards)
        if (m_events.onBoardCardsDealt)
        {
            m_events.onBoardCardsDealt(flopBoard);
        }
    }
    logStateInfo(hand);
}

// Helper method following Single Responsibility Principle
Logger& FlopState::getLogger()
{
    // Use focused dependency if available (ISP-compliant)
    if (m_loggerService) {
        return *m_loggerService;
    }
    
    // Fall back to base class implementation (Open/Closed Principle)
    static std::shared_ptr<pkt::core::ServiceContainer> defaultServices =
        std::make_shared<pkt::core::AppServiceContainer>();
    return defaultServices->logger();
}

// Override base class method to use focused dependency (Liskov Substitution Principle)
void FlopState::logStateInfo(Hand& hand)
{
    // Use ISP-compliant logging instead of hidden ServiceContainer dependency
    std::string boardStr = hand.getStringBoard();
    int pot = hand.getBoard().getPot(hand);
    
    Logger& logger = getLogger();
    logger.info(""); // Empty line for spacing
    logger.info("Current State: " + gameStateToString(hand.getGameState()) + 
               ", Board: " + boardStr + ", Pot: " + std::to_string(pot));
}

void FlopState::exit(Hand& hand)
{
}

bool FlopState::isActionAllowed(const Hand& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), 0, Flop));
}

void FlopState::promptPlayerAction(Hand& hand, Player& player)
{
    player.updateCurrentHandContext(hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<HandState> FlopState::computeNextState(Hand& hand)
{
    return computeBettingRoundNextState(hand, m_events, Flop, m_loggerService);
}

std::shared_ptr<player::Player> FlopState::getNextPlayerToAct(const Hand& hand) const
{
    return getNextPlayerToActInRound(hand, GameState::Flop);
}

std::shared_ptr<player::Player> FlopState::getFirstPlayerToActInRound(const Hand& hand) const
{
    return getNextPlayerToAct(hand);
}

bool FlopState::isRoundComplete(const Hand& hand) const
{
    return pkt::core::isRoundComplete(hand);
}

} // namespace pkt::core