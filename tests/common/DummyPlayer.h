#pragma once

#include <core/engine/GameEvents.h>
#include <core/interfaces/NullLogger.h>
#include <core/player/PlayerFsm.h>
#include <core/player/strategy/LooseAggressiveBotStrategy.h>
#include "DeterministicStrategy.h"

namespace pkt::test
{

class DummyPlayerFsm : public pkt::core::player::PlayerFsm
{
  public:
    DummyPlayerFsm(int id, const pkt::core::GameEvents& events)
        : pkt::core::player::PlayerFsm(events,                             // const GameEvents&
                                       id,                                 // Player ID
                                       "DummyBotFsm" + std::to_string(id), // Player Name
                                       1000)                               // starting cash
    {
        setStrategy(std::make_unique<pkt::test::DeterministicStrategy>());
    }
};

} // namespace pkt::test
