#pragma once
#include "Player.h"
#include "core/engine/model/ButtonState.h"

namespace pkt::core::player
{

class PlayerFsm
{
  public:
    explicit PlayerFsm(std::shared_ptr<Player>);

    unsigned int getId() const { return myId; }
    void setId(unsigned id) { myId = id; }

    std::shared_ptr<Player>& getLegacyPlayer() { return myPlayer; }

    void resetForNewHand();
    void setButton(Button);
    Button getButton() const;

  protected:
  private:
    std::shared_ptr<Player> myPlayer;
    Button myButton = ButtonNone;
    unsigned int myId;
};

} // namespace pkt::core::player