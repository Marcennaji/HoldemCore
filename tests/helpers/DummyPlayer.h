#pragma once
#include <core/engine/GameEvents.h>
#include <core/interfaces/NullLogger.h>
#include <core/player/BotPlayer.h>

namespace pkt::test
{

class DummyPlayer : public pkt::core::player::BotPlayer
{
  public:
    DummyPlayer(int id, const pkt::core::GameEvents& events)
        : pkt::core::player::BotPlayer(events,  // const GameEvents&
                                       id,      // Player ID
                                       "Dummy", // Player Name
                                       1000,    // Starting Cash
                                       true,    // Active Status
                                       0)       // Minimum Bet
    {
        setIsSessionActive(true);
    }
};

} // namespace pkt::test
