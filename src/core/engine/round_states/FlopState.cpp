#include "FlopState.h"
#include "GameEvents.h"
#include "HandFsm.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

FlopState::FlopState(const GameEvents& events) : myEvents(events)
{
}

void FlopState::enter(HandFsm& hand)
{
}

void FlopState::exit(HandFsm& hand)
{
    // Nothing needed for now
}

std::unique_ptr<IHandState> FlopState::processAction(HandFsm& hand, PlayerAction action)
{

    return nullptr;
}

bool FlopState::isRoundComplete(const HandFsm& hand) const
{
    return false;
}

bool FlopState::canProcessAction(const HandFsm& hand, PlayerAction action) const
{
    return false;
}

void FlopState::logStateInfo(const HandFsm& hand) const
{
    // Optional debug logging
}

} // namespace pkt::core