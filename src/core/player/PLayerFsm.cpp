#include "PlayerFsm.h"

namespace pkt::core::player
{
PlayerFsm::PlayerFsm(std::shared_ptr<Player> p) : myPlayer(p)
{
}
void PlayerFsm::resetForNewHand()
{
    myPlayer->setHand(nullptr);
    myPlayer->setCardsFlip(0);
}
} // namespace pkt::core::player