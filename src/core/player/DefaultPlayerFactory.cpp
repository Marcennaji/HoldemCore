#include "DefaultPlayerFactory.h"
#include "core/player/Player.h"
#include "core/player/strategy/BotStrategy.h"

namespace pkt::core::player
{

DefaultPlayerFactory::DefaultPlayerFactory(const GameEvents& events, StrategyAssigner* assigner)
    : m_events(events), m_strategyAssigner(assigner)
{
}

std::shared_ptr<Player> DefaultPlayerFactory::createPlayer(int id, TableProfile profile, int startMoney)
{
    // DefaultPlayerFactory should not be used in strict ISP mode as it doesn't have access to ISP interfaces
    // This factory is deprecated - use MixedPlayerFactory with proper ISP interface injection instead
    assert(false && "DefaultPlayerFactory is deprecated. Use MixedPlayerFactory with ISP interfaces instead.");
    return nullptr;
}
} // namespace pkt::core::player
