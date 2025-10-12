// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once
#include <memory>
#include "core/engine/model/TableProfile.h"
#include "core/ports/Logger.h"
#include "core/ports/Randomizer.h"
#include "core/player/strategy/BotStrategy.h"

namespace pkt::core::player
{

/**
 * @brief Assigns and manages AI strategies for bot players at the poker table.
 * 
 * Handles the allocation of different AI strategies to bot players based on
 * table configuration and desired playing styles, ensuring a diverse and
 * realistic playing environment for poker simulations.
 */
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
