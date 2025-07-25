#pragma once
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IHandState.h"
#include "core/interfaces/hand/IRoundCompletionChecker.h"

namespace pkt::core
{
class GameEvents;

class FlopState : public IHandState, public IActionProcessor, public IRoundCompletionChecker, public IDebuggableState
{
  public:
    FlopState(GameEvents& events);

    void enter(HandFsm& hand) override;
    void exit(HandFsm& hand) override;
    std::unique_ptr<IHandState> processAction(HandFsm& hand, PlayerAction action) override;

    bool isRoundComplete(const HandFsm& hand) const override;
    bool canProcessAction(const HandFsm& hand, PlayerAction action) const override;

    void logStateInfo(const HandFsm& hand) const override;
    std::string getStateName() const override { return "Flop"; }

  private:
    GameEvents& myEvents;
};

} // namespace pkt::core