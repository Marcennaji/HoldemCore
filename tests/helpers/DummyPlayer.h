#pragma once
#include <core/player/Player.h>

namespace pkt::test
{

class DummyPlayer : public pkt::core::player::Player
{
  public:
    DummyPlayer(int id = 42) : pkt::core::player::Player(nullptr, nullptr, nullptr, id, "Dummy", 1000, true, 0)
    {
        setIsSessionActive(true);
    }
};

} // namespace pkt::test
