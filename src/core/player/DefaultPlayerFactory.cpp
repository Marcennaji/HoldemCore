#include "DefaultPlayerFactory.h"
#include "core/player/PlayerFsm.h"
#include "core/player/strategy/BotStrategy.h"

namespace pkt::core::player
{

DefaultPlayerFactory::DefaultPlayerFactory(const GameEvents& events, StrategyAssigner* assigner)
    : myEvents(events), myStrategyAssigner(assigner)
{
}

std::shared_ptr<PlayerFsm> DefaultPlayerFactory::createPlayerFsm(int id, TableProfile profile, int startMoney)
{
    auto strategy = myStrategyAssigner->chooseStrategyFor(id);

    auto player = std::make_shared<PlayerFsm>(myEvents, id, "Bot_" + std::to_string(id), startMoney);
    player->setStrategy(std::move(strategy));
    return player;
}
} // namespace pkt::core::player
