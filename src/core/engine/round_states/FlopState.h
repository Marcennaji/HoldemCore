#pragma once
#include "core/interfaces/IBettingRoundStateFsm.h"

namespace pkt::core
{
class GameEvents;

class FlopState : public IBettingRoundStateFsm
{
  public:
    FlopState(GameEvents& events);

    void enter(IHand& hand) override;
    void exit(IHand& hand) override;
    std::unique_ptr<IBettingRoundStateFsm> processAction(IHand& hand, PlayerAction action) override;

    GameState getGameState() const override;
    std::string getStateName() const override;

    bool isRoundComplete(const IHand& hand) const override;
    bool canProcessAction(const IHand& hand, PlayerAction action) const override;

    void logStateInfo(const IHand& hand) const override;

  private:
    GameEvents& myEvents;
};

} // namespace pkt::core