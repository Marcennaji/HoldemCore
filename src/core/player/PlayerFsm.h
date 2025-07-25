#pragma once
#include "Player.h"

namespace pkt::core::player
{

class PlayerFsm
{
  public:
    explicit PlayerFsm(std::shared_ptr<Player>);

    std::shared_ptr<Player>& getLegacyPlayer() { return myPlayer; }

    void resetForNewHand();

  private:
    std::shared_ptr<Player> myPlayer;
};

} // namespace pkt::core::player