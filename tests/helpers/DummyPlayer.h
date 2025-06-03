#pragma once
#include <core/player/Player.h>

namespace pkt::test
{

class DummyPlayer : public pkt::core::Player
{
  public:
    DummyPlayer(int id = 42)
        : pkt::core::Player(nullptr, nullptr, nullptr, id, pkt::core::PLAYER_TYPE_COMPUTER, "Dummy", 1000, true, 0)
    {
        setIsSessionActive(true);
    }
};

} // namespace pkt::test
