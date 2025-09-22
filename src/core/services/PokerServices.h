// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "ServiceContainer.h"

namespace pkt::core
{

/**
 * @brief Specialized service container for poker game operations
 *
 * Provides services specifically needed by game engine classes like Hand, Game, etc.
 * Focuses on services like logging, hand evaluation, and randomization for game logic.
 */
class PokerServices : public ServiceContainer
{
  public:
    explicit PokerServices(std::shared_ptr<ServiceContainer> baseContainer);

    // Inherited service accessors
    ILogger& logger() override;
    PlayersStatisticsStore& playersStatisticsStore() override;
    HandEvaluationEngine& handEvaluationEngine() override;
    IRandomizer& randomizer() override;

  private:
    std::shared_ptr<ServiceContainer> myBaseContainer;
};

} // namespace pkt::core