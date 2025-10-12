#include "PreflopState.h"
#include "FlopState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "PostRiverState.h"
#include "core/engine/Exception.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/engine/utils/Helpers.h"
#include "core/player/Player.h"

namespace pkt::core
{
using namespace pkt::core::player;

// ISP-compliant constructor using focused service interface
PreflopState::PreflopState(const GameEvents& events, const int smallBlind, unsigned dealerPlayerId,
                           Logger& logger)
    : m_events(events), m_smallBlind(smallBlind), m_dealerPlayerId(dealerPlayerId), m_logger(logger)
{
    if (smallBlind <= 0)
    {
        throw std::invalid_argument("PreflopState: smallBlind must be > 0");
    }

    if (dealerPlayerId == static_cast<unsigned>(-1))
    {
        throw std::invalid_argument("PreflopState: dealerPlayerId is invalid");
    }
}

void PreflopState::enter(Hand& hand)
{
    for (auto& player : *hand.getSeatsList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    hand.getBettingActions()->updateRoundHighestSet(2 * m_smallBlind);
    setBlinds(hand);

    if (m_events.onBettingRoundStarted)
        m_events.onBettingRoundStarted(Preflop);

    logStateInfo(hand);
}

void PreflopState::exit(Hand& hand)
{
}

bool PreflopState::isActionAllowed(const Hand& hand, const PlayerAction action) const
{
    return (
        validatePlayerAction(hand.getActingPlayersList(), action, *hand.getBettingActions(), m_smallBlind, Preflop));
}

void PreflopState::promptPlayerAction(Hand& hand, Player& player)
{
    player.updateCurrentHandContext(hand);
    PlayerAction action = player.decideAction(player.getCurrentHandContext());

    hand.handlePlayerAction(action);
}

std::unique_ptr<HandState> PreflopState::computeNextState(Hand& hand)
{
    return computeBettingRoundNextState(hand, m_events, Preflop, m_logger);
}

void PreflopState::logStateInfo(Hand& hand)
{
    m_logger.info("Blinds: SB=" + std::to_string(m_smallBlind) +
                     ", BB=" + std::to_string(2 * m_smallBlind));
    // Log dealer and positions to help diagnose acting order
    m_logger.info("Dealer: Player " + std::to_string(hand.getDealerPlayerId()));
    for (const auto& player : *hand.getSeatsList()) {
        m_logger.info(
            "Player " + std::to_string(player->getId()) + " (" + player->getName() + ") position=" +
            positionToString(player->getPosition()));
    }
    logHoleCards(hand);
}

void PreflopState::logHoleCards(Hand& hand)
{
    for (const auto& player : *hand.getSeatsList())
    {
        const HoleCards& holeCards = player->getHoleCards();
        if (holeCards.isValid())
        {
            m_logger.info("Player " + std::to_string(player->getId()) + " (" + player->getName() +
                            "): " + holeCards.toString());
        }
        else
        {
            m_logger.info("Player " + std::to_string(player->getId()) + " (" + player->getName() +
                            "): No hole cards");
        }
    }
}

std::shared_ptr<player::Player> PreflopState::getNextPlayerToAct(const Hand& hand) const
{
    return getNextPlayerToActInRound(hand, GameState::Preflop);
}

std::shared_ptr<player::Player> PreflopState::getFirstPlayerToActInRound(const Hand& hand) const
{
    // In preflop, the first player to act is left of the big blind
    // This handles both heads-up (SB acts first) and multi-player (UTG acts first)
    return getNextPlayerToAct(hand);
}

bool PreflopState::isRoundComplete(const Hand& hand) const
{
    return pkt::core::isRoundComplete(hand, m_logger);
}

void PreflopState::setBlinds(Hand& hand)
{
    for (const auto& player : *hand.getActingPlayersList())
    {
        int blindAmount = 0;
        ActionType actionType = ActionType::None;

        if (player->getPosition() == PlayerPosition::SmallBlind ||
            player->getPosition() == PlayerPosition::ButtonSmallBlind)
        {
            blindAmount = m_smallBlind;
            actionType = ActionType::PostSmallBlind;
        }
        else if (player->getPosition() == PlayerPosition::BigBlind)
        {
            blindAmount = 2 * m_smallBlind;
            actionType = ActionType::PostBigBlind;
        }

        if (blindAmount > 0)
        {
            PlayerAction blindAction;
            if (player->getCash() < blindAmount)
            {
                // Player doesn't have enough cash to post the blind, goes all-in
                const int remaining = player->getCash();
                player->addBetAmount(remaining);
                blindAction = {player->getId(), ActionType::Allin, remaining};
                player->setAction(*this, blindAction);
            }
            else
            {
                // Player can afford the blind
                player->addBetAmount(blindAmount);
                blindAction = {player->getId(), actionType, blindAmount};
                player->setAction(*this, blindAction);
            }

            // Record blind post in hand-level chronological history
            hand.getBettingActions()->recordPlayerAction(getGameState(), blindAction);

            if (m_events.onPlayerActed)
            {
                m_events.onPlayerActed(blindAction);
            }
        }
    }
}
} // namespace pkt::core
