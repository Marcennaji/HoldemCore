#include "DefaultPlayerFactory.h"
#include <core/player/deprecated/BotPlayer.h>
#include "core/player/deprecated/HumanPlayer.h"
#include "core/player/strategy/BotStrategy.h"

namespace pkt::core::player
{

DefaultPlayerFactory::DefaultPlayerFactory(const GameEvents& events, StrategyAssigner* assigner)
    : myEvents(events), myStrategyAssigner(assigner)
{
}

std::shared_ptr<Player> DefaultPlayerFactory::createHumanPlayer(int id, int startMoney)
{
    return std::make_shared<HumanPlayer>(myEvents, id, startMoney, true, 0);
}

std::shared_ptr<Player> DefaultPlayerFactory::createBotPlayer(int id, TableProfile profile, int startMoney)
{
    auto strategy = myStrategyAssigner->chooseStrategyFor(id);

    auto bot = std::make_shared<BotPlayer>(myEvents, id, "Bot_" + std::to_string(id), startMoney, true, 0);
    bot->setStrategy(std::move(strategy));
    return bot;
}

} // namespace pkt::core::player
