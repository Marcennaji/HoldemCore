#pragma once
#include <memory>
#include "core/engine/model/TableProfile.h"
#include "core/interfaces/Logger.h"
#include "core/interfaces/Randomizer.h"
#include "core/player/strategy/BotStrategy.h"

namespace pkt::core::player
{

class StrategyAssigner
{
  public:

    StrategyAssigner(TableProfile profile, int botCount, 
                     pkt::core::Logger& logger,
                     pkt::core::Randomizer& randomizer);

    std::unique_ptr<BotStrategy> chooseBotStrategyFor(int botIndex);

  private:
    TableProfile m_profile;
    int m_countManiac = 0;
    int m_countLoose = 0;
    int m_countTight = 0;
    int m_countUltraTight = 0;
    int m_maxPerType;
    
    pkt::core::Logger* m_logger;
    pkt::core::Randomizer* m_randomizer;
};

} // namespace pkt::core::player
