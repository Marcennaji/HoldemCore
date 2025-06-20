#pragma once
#include <memory>
#include "core/engine/model/TableProfile.h"
#include "core/interfaces/ILogger.h"
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
    int myCountManiac = 0;
    int myCountLoose = 0;
    int myCountTight = 0;
    int myCountUltraTight = 0;
    int myMaxPerType;
};

} // namespace pkt::core::player
