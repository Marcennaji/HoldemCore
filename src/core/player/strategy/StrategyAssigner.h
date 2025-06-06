#pragma once
#include <memory>
#include "core/engine/model/TableProfile.h"
#include "core/player/strategy/IBotStrategy.h"

namespace pkt::core::player
{

class StrategyAssigner
{
  public:
    StrategyAssigner(TableProfile profile, int botCount);

    std::unique_ptr<IBotStrategy> chooseStrategyFor(int botIndex);

  private:
    TableProfile myProfile;
    int countManiac = 0;
    int countLoose = 0;
    int countTight = 0;
    int countUltraTight = 0;
    int maxPerType;
};

} // namespace pkt::core::player
