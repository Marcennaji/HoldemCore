#pragma once
#include <core/engine/GameEvents.h>
#include <core/interfaces/NullLogger.h>
#include <core/player/BotPlayer.h>
#include <core/player/BotPlayerFsm.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>

namespace pkt::test
{

class DummyPlayer : public pkt::core::player::BotPlayer
{
  public:
    DummyPlayer(int id, const pkt::core::GameEvents& events)
        : pkt::core::player::BotPlayer(events,                          // const GameEvents&
                                       id,                              // Player ID
                                       "DummyBot" + std::to_string(id), // Player Name
                                       1000,                            // Starting Cash
                                       true,                            // Active Status
                                       0)                               // Minimum Bet
    {
        setStrategy(std::make_unique<pkt::core::player::LooseAggressiveBotStrategy>());
    }
};

class DummyPlayerFsm : public pkt::core::player::BotPlayerFsm
{
  public:
    DummyPlayerFsm(int id, const pkt::core::GameEvents& events)
        : pkt::core::player::BotPlayerFsm(events,                             // const GameEvents&
                                          id,                                 // Player ID
                                          "DummyBotFsm" + std::to_string(id), // Player Name
                                          1000)                               // starting cash
    {
        setStrategy(std::make_unique<pkt::core::player::LooseAggressiveBotStrategy>());
    }
};

} // namespace pkt::test
