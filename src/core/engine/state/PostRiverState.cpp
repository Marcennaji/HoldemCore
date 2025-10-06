#include "PostRiverState.h"
#include "GameEvents.h"
#include "Hand.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Helpers.h"
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{
using namespace pkt::core::player;

PostRiverState::PostRiverState(const GameEvents& events) : m_events(events)
{
}

PostRiverState::PostRiverState(const GameEvents& events, std::shared_ptr<pkt::core::ServiceContainer> services)
    : m_events(events), m_services(std::move(services))
{
}

// ISP-compliant constructor using focused service interface
PostRiverState::PostRiverState(const GameEvents& events, std::shared_ptr<Logger> logger)
    : m_events(events), m_logger(logger)
{
}

// ISP-compliant helper method
pkt::core::Logger& PostRiverState::getLogger() const
{
    if (m_logger) {
        return *m_logger;
    }
    // This should not happen in normal operation
    throw std::runtime_error("PostRiverState: Logger service not properly initialized. Use ISP-compliant constructor.");
}

void PostRiverState::enter(Hand& hand)
{
    getLogger().info("Post-River");

    for (auto& player : *hand.getActingPlayersList())
    {
        player->setAction(*this, {player->getId(), ActionType::None});
    }

    hand.getBoard().distributePot(hand);

    // Compute the exact showdown reveal sequence (engine domain logic)
    hand.getBoard().determineShowdownRevealOrder();
    if (m_events.onShowdownRevealOrder)
    {
        auto order = hand.getBoard().getShowdownRevealOrder();
        m_events.onShowdownRevealOrder(order);
    }

    if (m_events.onBettingRoundStarted)
        m_events.onBettingRoundStarted(PostRiver);

    logStateInfo(hand);
}

void PostRiverState::exit(Hand& hand)
{
    // Clear betting data after pot distribution is complete
    for (auto& player : *hand.getSeatsList())
    {
        player->resetCurrentHandActions();
    }
}

std::unique_ptr<HandState> PostRiverState::computeNextState(Hand& /*hand*/)
{
    // PostRiver is terminal — no next state.
    return nullptr;
}

bool PostRiverState::isActionAllowed(const Hand& /*hand*/, const PlayerAction /*action*/) const
{
    // No player actions allowed in PostRiver.
    return false;
}

void PostRiverState::promptPlayerAction(Hand& /*hand*/, player::Player& /*player*/)
{
    // No player actions in PostRiver.
}

std::shared_ptr<player::Player> PostRiverState::getNextPlayerToAct(const Hand& /*hand*/) const
{
    return nullptr;
}

std::shared_ptr<player::Player> PostRiverState::getFirstPlayerToActInRound(const Hand& /*hand*/) const
{
    return nullptr;
}

bool PostRiverState::isRoundComplete(const Hand& /*hand*/) const
{
    return true;
}

} // namespace pkt::core