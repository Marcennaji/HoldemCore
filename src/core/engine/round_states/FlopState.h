#pragma once
#include "core/engine/round_states/BettingStateBase.h"
#include "core/interfaces/hand/IActionProcessor.h"
#include "core/interfaces/hand/IDebuggableState.h"
#include "core/interfaces/hand/IRoundCompletionChecker.h"

namespace pkt::core
{
class GameEvents;

class FlopState : public BettingStateBase,
                  public IActionProcessor,
                  public IRoundCompletionChecker,
                  public IDebuggableState
{
  public:
    FlopState(const GameEvents& events);

    void enter(HandFsm& hand) override;
    void exit(HandFsm& hand) override;
    std::unique_ptr<IHandState> computeNextState(HandFsm& hand, PlayerAction action) override;

    bool isRoundComplete(const HandFsm& hand) const override;
    bool isActionAllowed(const HandFsm& hand, PlayerAction action) const override;

    void logStateInfo(const HandFsm& hand) const override;
    std::string getStateName() const override { return "Flop"; }

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core