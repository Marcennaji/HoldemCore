

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

std::vector<ActionType>& CurrentHandActions::getPreflopActions()
{
    return m_preflopActions;
}

std::vector<ActionType>& CurrentHandActions::getFlopActions()
{
    return m_flopActions;
}

std::vector<ActionType>& CurrentHandActions::getTurnActions()
{
    return m_turnActions;
}

std::vector<ActionType>& CurrentHandActions::getRiverActions()
{
    return m_riverActions;
}
} // namespace pkt::core::player
