#include "DefaultPlayerFactory.h"
#include "BotPlayer.h"
#include "HumanPlayer.h"
#include "core/player/strategy/IBotStrategy.h"

namespace pkt::core::player
{

DefaultPlayerFactory::DefaultPlayerFactory(GameEvents* events, IHandAuditStore* audit, IPlayersStatisticsStore* stats,
                                           StrategyAssigner* assigner)
    : myEvents(events), myAudit(audit), myStats(stats), myStrategyAssigner(assigner)
{
}

std::shared_ptr<Player> DefaultPlayerFactory::createHumanPlayer(int id, const std::string& name, int startMoney)
{
    return std::make_shared<HumanPlayer>(myEvents, myAudit, myStats, id, PLAYER_TYPE_HUMAN, name, startMoney, true, 0);
}

std::shared_ptr<Player> DefaultPlayerFactory::createBotPlayer(int id, TableProfile profile)
{
    auto strategy = myStrategyAssigner->chooseStrategyFor(id);

    auto bot = std::make_shared<BotPlayer>(myEvents, myAudit, myStats, id, PLAYER_TYPE_COMPUTER,
                                           "Bot_" + std::to_string(id), 1000, true, 0);
    bot->setStrategy(std::move(strategy));
    return bot;
}

} // namespace pkt::core::player
