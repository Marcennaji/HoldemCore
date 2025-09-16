#pragma once

#include <core/engine/GameEvents.h>
#include <core/interfaces/NullLogger.h>
#include <core/player/Player.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include "DeterministicStrategy.h"

namespace pkt::test
{

class DummyPlayer : public pkt::core::player::Player
{
  public:
    DummyPlayer(int id, const pkt::core::GameEvents& events)
        : pkt::core::player::Player(events,                     // const GameEvents&
                                    id,                         // Player ID
                                    "Bot" + std::to_string(id), // Player Name
                                    1000)                       // starting cash
    {
        setStrategy(std::make_unique<pkt::test::DeterministicStrategy>());
    }
};

} // namespace pkt::test
