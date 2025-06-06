#pragma once

#include "IPlayerFactory.h"
#include "core/player/strategy/StrategyAssigner.h"

namespace pkt::core::player
{

class DefaultPlayerFactory : public IPlayerFactory
{
  public:
    DefaultPlayerFactory(GameEvents* events, IHandAuditStore* audit, IPlayersStatisticsStore* stats,
                         StrategyAssigner* assigner);

    std::shared_ptr<Player> createHumanPlayer(int id, const std::string& name, int startMoney) override;
    std::shared_ptr<Player> createBotPlayer(int id, TableProfile profile) override;

  private:
    GameEvents* myEvents;
    IHandAuditStore* myAudit;
    IPlayersStatisticsStore* myStats;
    StrategyAssigner* myStrategyAssigner;
};
} // namespace pkt::core::player
