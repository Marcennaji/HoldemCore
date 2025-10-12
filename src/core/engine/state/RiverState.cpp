#include "RiverState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/utils/Helpers.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace pkt::core::player;


RiverState::RiverState(const GameEvents& events, Logger& logger)
    : m_events(events), m_logger(logger)
{
}

void RiverState::enter(Hand& hand)
{
    m_logger.info("River");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    // Reset betting amounts for new round
    hand.getBettingActions()->resetRoundHighestSet();

    if (m_events.onBettingRoundStarted)
        m_events.onBettingRoundStarted(River);

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
        if (m_events.onBoardCardsDealt)
        {
            m_events.onBoardCardsDealt(riverBoard);
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

std::unique_ptr<HandState> RiverState::computeNextState(Hand& hand)
{
    return computeBettingRoundNextState(hand, m_events, River, m_logger);
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
    return pkt::core::isRoundComplete(hand, m_logger);
}

void RiverState::logStateInfo(Hand& hand)
{
    m_logger.info("River state - fifth community card dealt");
}

} // namespace pkt::core