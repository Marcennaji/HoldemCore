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

    void enter(IHand& hand) override;
    void exit(IHand& hand) override;
    std::unique_ptr<IHandState> processAction(IHand& hand, PlayerAction action) override;

    bool isRoundComplete(const IHand& hand) const override;
    bool canProcessAction(const IHand& hand, PlayerAction action) const override;

    void logStateInfo(const IHand& hand) const override;
    std::string getStateName() const override { return "Flop"; }

  private:
    GameEvents& myEvents;
};

} // namespace pkt::core