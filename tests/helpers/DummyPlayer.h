#pragma once
#include <core/player/BotPlayer.h>

namespace pkt::test
{

class DummyPlayer : public pkt::core::player::BotPlayer
{
  public:
    DummyPlayer(int id = 42) : pkt::core::player::BotPlayer(nullptr, nullptr, nullptr, id, "Dummy", 1000, true, 0)
    {
        setIsSessionActive(true);
    }
};

} // namespace pkt::test
