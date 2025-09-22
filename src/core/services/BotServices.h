// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "ServiceContainer.h"

namespace pkt::core
{

/**
 * @brief Specialized service container for bot-related operations
 *
 * Provides services specifically needed by bot strategy classes and player logic.
 * Focuses on services like hand evaluation, randomization, and statistics.
 */
class BotServices : public ServiceContainer
{
  public:
    explicit BotServices(std::shared_ptr<ServiceContainer> baseContainer);

    // Inherited service accessors
    ILogger& logger() override;
    PlayersStatisticsStore& playersStatisticsStore() override;
    HandEvaluationEngine& handEvaluationEngine() override;
    IRandomizer& randomizer() override;

  private:
    std::shared_ptr<ServiceContainer> myBaseContainer;
};

} // namespace pkt::core