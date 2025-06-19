#pragma once
#include <core/interfaces/NullLogger.h>
#include <core/player/BotPlayer.h>

namespace pkt::test
{

class DummyPlayer : public pkt::core::player::BotPlayer
{
  public:
    DummyPlayer(int id = 42)
        : pkt::core::player::BotPlayer(nullptr,                                         // GameEvents*
                                       std::make_shared<pkt::core::NullLogger>().get(), // ILogger*
                                       nullptr,                                         // IHandAuditStore*
                                       nullptr,                                         // IPlayersStatisticsStore*
                                       id,                                              // Player ID
                                       "Dummy",                                         // Player Name
                                       1000,                                            // Starting Cash
                                       true,                                            // Active Status
                                       0)                                               // Minimum Bet
    {
        setIsSessionActive(true);
    }
};

} // namespace pkt::test
