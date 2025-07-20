#pragma once
#include "core/interfaces/IBettingRoundStateFsm.h"

namespace pkt::core
{

class GameEvents;

class PreflopState : public IBettingRoundStateFsm
{
  public:
    PreflopState(GameEvents& events);

    void enter(IHand& IHand) override;
    void exit(IHand& IHand) override;
    std::unique_ptr<IBettingRoundStateFsm> processAction(IHand& IHand, PlayerAction action) override;

    GameState getGameState() const override;
    std::string getStateName() const override;

    bool isRoundComplete(const IHand& IHand) const override;
    bool canProcessAction(const IHand& IHand, PlayerAction action) const override;

    void logStateInfo(const IHand& IHand) const override;

  private:
    GameEvents& myEvents;
};

} // namespace pkt::core