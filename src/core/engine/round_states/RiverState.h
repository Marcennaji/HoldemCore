#pragma once
#include "core/interfaces/IBettingRoundStateFsm.h"

namespace pkt::core
{

class GameEvents;

class RiverState : public IBettingRoundStateFsm
{
  public:
    RiverState(GameEvents& events);

    void enter(IHand& Ihand) override;
    void exit(IHand& Ihand) override;
    std::unique_ptr<IBettingRoundStateFsm> processAction(IHand& Ihand, PlayerAction action) override;

    GameState getGameState() const override;
    std::string getStateName() const override;

    bool isRoundComplete(const IHand& Ihand) const override;
    bool canProcessAction(const IHand& Ihand, PlayerAction action) const override;

    void logStateInfo(const IHand& Ihand) const override;

  private:
    GameEvents& myEvents;
};

} // namespace pkt::core