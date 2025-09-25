#include "RiverState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/utils/Helpers.h"
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{
using namespace pkt::core::player;

RiverState::RiverState(const GameEvents& events) : myEvents(events)
{
}

RiverState::RiverState(const GameEvents& events, std::shared_ptr<pkt::core::ServiceContainer> services)
    : myEvents(events), myServices(std::move(services))
{
}

void RiverState::ensureServicesInitialized() const
{
    if (!myServices)
    {
        static std::shared_ptr<pkt::core::ServiceContainer> defaultServices =
            std::make_shared<pkt::core::AppServiceContainer>();
        myServices = defaultServices;
    }
}

void RiverState::enter(Hand& hand)
{
    ensureServicesInitialized();
    myServices->logger().info("River");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(River);

    // Deal river card and fire event with river-specific BoardCards
    BoardCards currentBoard = hand.getBoard().getBoardCards();
    if (currentBoard.getNumCards() == 4) // Only deal river if we have exactly 4 cards (turn)
    {
        // Get next card from deck for river
        std::vector<Card> riverCards = hand.dealCardsFromDeck(1);

        // Create river-specific BoardCards (5 cards)
        BoardCards riverBoard = currentBoard; // Copy current turn board
        riverBoard.dealRiver(riverCards[0]);  // Add river card

        // Update board with river card
        hand.getBoard().setBoardCards(riverBoard);

        // Fire event with river-specific board (5 cards)
        if (myEvents.onBoardCardsDealt)
        {
            myEvents.onBoardCardsDealt(riverBoard);
        }
    }
    logStateInfo(hand);
}

void RiverState::exit(Hand& hand)
{
}

bool RiverState::isActionAllowed(const Hand& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), 0, River));
}

void RiverState::promptPlayerAction(Hand& hand, Player& player)
{
    player.updateCurrentHandContext(hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> RiverState::computeNextState(Hand& hand)
{
    return computeBettingRoundNextState(hand, myEvents, River);
}

std::shared_ptr<player::Player> RiverState::getNextPlayerToAct(const Hand& hand) const
{
    return getNextPlayerToActInRound(hand, GameState::River);
}

std::shared_ptr<player::Player> RiverState::getFirstPlayerToActInRound(const Hand& hand) const
{
    return getNextPlayerToAct(hand);
}
bool RiverState::isRoundComplete(const Hand& hand) const
{
    return pkt::core::isRoundComplete(hand);
}

} // namespace pkt::core