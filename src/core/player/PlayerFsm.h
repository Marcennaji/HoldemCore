#pragma once
#include "Player.h"

namespace pkt::core::player
{

class PlayerFsm
{
  public:
    explicit PlayerFsm(Player&);

    Player& getLegacyPlayer() const { return myPlayer; }

  private:
    Player& myPlayer;
};

} // namespace pkt::core::player