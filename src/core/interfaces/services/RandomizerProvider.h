// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Randomizer.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Focused interface for classes that need randomization capabilities
 * 
 * Replaces the monolithic ServiceContainer dependency with a focused
 * interface that only provides randomizer access, following ISP.
 */
class RandomizerProvider
{
  public:
    virtual ~RandomizerProvider() = default;
    virtual Randomizer& getRandomizer() = 0;
};

/**
 * @brief Simple implementation that wraps a Randomizer instance
 */
class SimpleRandomizerProvider : public RandomizerProvider
{
  public:
    explicit SimpleRandomizerProvider(std::shared_ptr<Randomizer> randomizer) : m_randomizer(randomizer) {}
    
    Randomizer& getRandomizer() override { return *m_randomizer; }
    
  private:
    std::shared_ptr<Randomizer> m_randomizer;
};

} // namespace pkt::core