#include "PlayerFsm.h"

namespace pkt::core::player
{
PlayerFsm::PlayerFsm(std::shared_ptr<Player> p) : myPlayer(p)
{
    setId(myPlayer->getId());
}
void PlayerFsm::resetForNewHand()
{
    myPlayer->setHand(nullptr);
    myPlayer->setCardsFlip(0);
    myPlayer->getCurrentHandActions().reset();
}
void PlayerFsm::setButton(Button b)
{
    myButton = b;
}
Button PlayerFsm::getButton() const
{
    return myButton;
}
} // namespace pkt::core::player