#pragma once

#include <core/engine/GameEvents.h>
#include <core/interfaces/NullLogger.h>
#include <core/player/Player.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include <core/services/ServiceContainer.h>
#include <core/interfaces/ServiceAdapter.h>
#include <core/interfaces/Randomizer.h>
#include "DeterministicStrategy.h"

namespace pkt::test
{

class DummyPlayer : public pkt::core::player::Player
{
  public:
    // ISP-compliant constructor using ServiceContainer (for existing test infrastructure)
    DummyPlayer(int id, const pkt::core::GameEvents& events, std::shared_ptr<pkt::core::ServiceContainer> services)
        : pkt::core::player::Player(events, 
                                   std::make_shared<pkt::core::ServiceAdapter>(services)->createLoggerService(),
                                   std::make_shared<pkt::core::ServiceAdapter>(services)->createHandEvaluationEngineService(),
                                   std::make_shared<pkt::core::ServiceAdapter>(services)->createPlayersStatisticsStoreService(),
                                   std::make_shared<pkt::core::ServiceAdapter>(services)->createRandomizerService(),
                                   id, "Bot" + std::to_string(id), 1000)
    {
        setStrategy(std::make_unique<pkt::test::DeterministicStrategy>());
    }
    
    // Fully ISP-compliant constructor with direct interface injection
    DummyPlayer(int id, const pkt::core::GameEvents& events, 
                std::shared_ptr<pkt::core::Logger> logger,
                std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluator,
                std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                std::shared_ptr<pkt::core::Randomizer> randomizer)
        : pkt::core::player::Player(events, logger, handEvaluator, statisticsStore, randomizer, id, "Bot" + std::to_string(id), 1000)
    {
        setStrategy(std::make_unique<pkt::test::DeterministicStrategy>());
    }
};

} // namespace pkt::test
