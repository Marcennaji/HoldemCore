#include "HandFsm.h"
#include "core/engine/round_states/PreflopState.h"
#include "core/interfaces/IHand.h"
#include "core/interfaces/hand/IHandState.h"

namespace pkt::core
{

HandFsm::HandFsm() = default;
HandFsm::~HandFsm() = default;

void HandFsm::startHand()
{
    // myState = std::make_unique<PreflopState>(getEvents());
}

void HandFsm::endHand()
{
    myState.reset();
}

void HandFsm::handlePlayerAction(int playerId, PlayerAction action)
{
    if (!myState)
        return;

    if (auto* processor = dynamic_cast<IActionProcessor*>(myState.get()))
    {
        if (!processor->canProcessAction(*this, action))
        {
            // Handle invalid action
            return;
        }

        auto next = processor->processAction(*this, action);
        if (next)
        {
            myState->exit(*this);
            myState = std::move(next);
            myState->enter(*this);
        }
    }
}

} // namespace pkt::core
