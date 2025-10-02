#include "TurnState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "RiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/utils/Helpers.h"
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{
using namespace pkt::core::player;

TurnState::TurnState(const GameEvents& events) : m_events(events)
{
}

TurnState::TurnState(const GameEvents& events, std::shared_ptr<pkt::core::ServiceContainer> services)
    : m_events(events), m_services(std::move(services))
{
}

void TurnState::ensureServicesInitialized() const
{
    if (!m_services)
    {
        static std::shared_ptr<pkt::core::ServiceContainer> defaultServices =
            std::make_shared<pkt::core::AppServiceContainer>();
        m_services = defaultServices;
    }
}

void TurnState::enter(Hand& hand)
{
    ensureServicesInitialized();
    m_services->logger().info("Turn");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }
    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    if (m_events.onBettingRoundStarted)
        m_events.onBettingRoundStarted(Turn);

    // Deal turn card and fire event with turn-specific BoardCards
    BoardCards currentBoard = hand.getBoard().getBoardCards();
    if (currentBoard.getNumCards() == 3) // Only deal turn if we have exactly 3 cards (flop)
    {
        // Get next card from deck for turn
        std::vector<Card> turnCards = hand.dealCardsFromDeck(1);

        // Create turn-specific BoardCards (4 cards)
        BoardCards turnBoard = currentBoard; // Copy current flop
        turnBoard.dealTurn(turnCards[0]);    // Add turn card

        // Update board with turn card
        hand.getBoard().setBoardCards(turnBoard);

        // Fire event with turn-specific board (4 cards)
        if (m_events.onBoardCardsDealt)
        {
            m_events.onBoardCardsDealt(turnBoard);
        }
    }
    logStateInfo(hand);
}

void TurnState::exit(Hand& hand)
{
}

bool TurnState::isActionAllowed(const Hand& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), 0, Turn));
}

void TurnState::promptPlayerAction(Hand& hand, Player& player)
{
    player.updateCurrentHandContext(hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<HandState> TurnState::computeNextState(Hand& hand)
{
    return computeBettingRoundNextState(hand, m_events, Turn);
}

std::shared_ptr<player::Player> TurnState::getNextPlayerToAct(const Hand& hand) const
{
    return getNextPlayerToActInRound(hand, GameState::Turn);
}

std::shared_ptr<player::Player> TurnState::getFirstPlayerToActInRound(const Hand& hand) const
{
    // In post-flop rounds, the first player to act is left of the dealer
    return getNextPlayerToAct(hand);
}
bool TurnState::isRoundComplete(const Hand& hand) const
{
    return pkt::core::isRoundComplete(hand);
}

} // namespace pkt::core