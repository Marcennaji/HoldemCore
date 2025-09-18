#include "TurnState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "RiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/utils/Helpers.h"
#include "core/player/Player.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace pkt::core::player;

TurnState::TurnState(const GameEvents& events) : myEvents(events)
{
}

void TurnState::enter(Hand& hand)
{
    GlobalServices::instance().logger().info("Turn");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }
    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    if (myEvents.onBettingRoundStarted)
        myEvents.onBettingRoundStarted(Turn);

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
        if (myEvents.onBoardCardsDealt)
        {
            myEvents.onBoardCardsDealt(turnBoard);
        }
    }
    logStateInfo(hand);
}

void TurnState::exit(Hand& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->updateCurrentHandContext(GameState::Turn, hand);
        player->getStatisticsUpdater()->updateTurnStatistics(player->getCurrentHandContext());
    }
}

bool TurnState::isActionAllowed(const Hand& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), 0, Turn));
}

void TurnState::promptPlayerAction(Hand& hand, Player& player)
{
    player.updateCurrentHandContext(Turn, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> TurnState::computeNextState(Hand& hand)
{
    return computeBettingRoundNextState(hand, myEvents, Turn);
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