
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

    std::vector<ActionType>& getPreflopActions();
    std::vector<ActionType>& getFlopActions();
    std::vector<ActionType>& getTurnActions();
    std::vector<ActionType>& getRiverActions();

    const std::vector<ActionType>& getPreflopActions() const { return m_preflopActions; };
    const std::vector<ActionType>& getFlopActions() const { return m_flopActions; };
    const std::vector<ActionType>& getTurnActions() const { return m_turnActions; };
    const std::vector<ActionType>& getRiverActions() const { return m_riverActions; };

  protected:
    std::vector<ActionType> m_preflopActions;
    std::vector<ActionType> m_flopActions;
    std::vector<ActionType> m_turnActions;
    std::vector<ActionType> m_riverActions;
};
} // namespace pkt::core::player