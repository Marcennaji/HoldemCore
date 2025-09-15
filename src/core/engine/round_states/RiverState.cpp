#include "RiverState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "PostRiverState.h"
#include "core/engine/Helpers.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/PlayerFsm.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{
using namespace pkt::core::player;

RiverState::RiverState(const GameEvents& events) : myEvents(events)
{
}

void RiverState::enter(HandFsm& hand)
{
    GlobalServices::instance().logger().info("River");

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

void RiverState::exit(HandFsm& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->updateCurrentHandContext(GameState::River, hand);
        player->getStatisticsUpdater()->updateRiverStatistics(player->getCurrentHandContext());
    }
}

bool RiverState::isActionAllowed(const HandFsm& hand, const PlayerAction action) const
{
    return (validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), 0, River));
}

void RiverState::promptPlayerAction(HandFsm& hand, PlayerFsm& player)
{
    player.updateCurrentHandContext(River, hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<IHandState> RiverState::computeNextState(HandFsm& hand)
{
    return computeBettingRoundNextState(hand, myEvents, River);
}

std::shared_ptr<player::PlayerFsm> RiverState::getNextPlayerToAct(const HandFsm& hand) const
{
    return getNextPlayerToActInRound(hand, GameState::River);
}

std::shared_ptr<player::PlayerFsm> RiverState::getFirstPlayerToActInRound(const HandFsm& hand) const
{
    return getNextPlayerToAct(hand);
}
bool RiverState::isRoundComplete(const HandFsm& hand) const
{
    return pkt::core::isRoundComplete(const_cast<HandFsm&>(hand));
}

} // namespace pkt::core