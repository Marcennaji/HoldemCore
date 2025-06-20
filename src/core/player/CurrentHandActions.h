
#pragma once

#include <core/engine/model/PlayerAction.h>

#include <vector>

namespace pkt::core::player
{

class CurrentHandActions
{
  public:
    CurrentHandActions() { reset(); };
    ~CurrentHandActions() = default;

    void reset(); // init to zero

    std::vector<PlayerAction>& getPreflopActions();
    std::vector<PlayerAction>& getFlopActions();
    std::vector<PlayerAction>& getTurnActions();
    std::vector<PlayerAction>& getRiverActions();

    const std::vector<PlayerAction>& getPreflopActions() const { return m_preflopActions; };
    const std::vector<PlayerAction>& getFlopActions() const { return m_flopActions; };
    const std::vector<PlayerAction>& getTurnActions() const { return m_turnActions; };
    const std::vector<PlayerAction>& getRiverActions() const { return m_riverActions; };

  protected:
    friend class Player;
    friend class HumanPlayer;
    friend class BotPlayer;

    std::vector<PlayerAction> m_preflopActions;
    std::vector<PlayerAction> m_flopActions;
    std::vector<PlayerAction> m_turnActions;
    std::vector<PlayerAction> m_riverActions;
};
} // namespace pkt::core::player