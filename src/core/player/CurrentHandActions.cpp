

#include "CurrentHandActions.h"

namespace pkt::core::player
{

void CurrentHandActions::reset()
{

    m_preflopActions.clear();
    m_flopActions.clear();
    m_turnActions.clear();
    m_riverActions.clear();
}

std::vector<PlayerAction>& CurrentHandActions::getPreflopActions()
{
    return m_preflopActions;
}

std::vector<PlayerAction>& CurrentHandActions::getFlopActions()
{
    return m_flopActions;
}

std::vector<PlayerAction>& CurrentHandActions::getTurnActions()
{
    return m_turnActions;
}

std::vector<PlayerAction>& CurrentHandActions::getRiverActions()
{
    return m_riverActions;
}
} // namespace pkt::core::player
