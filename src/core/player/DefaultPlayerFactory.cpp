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
    auto strategy = m_strategyAssigner->chooseBotStrategyFor(id);

    auto player = std::make_shared<Player>(m_events, id, "Bot_" + std::to_string(id), startMoney);
    player->setStrategy(std::move(strategy));
    return player;
}
} // namespace pkt::core::player
