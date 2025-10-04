// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/Randomizer.h"
#include <memory>

namespace pkt::core
{

/**
 * @brief Simple interface for components that only need randomization
 * 
 * This replaces the heavy ServiceContainer dependency with a focused interface.
 * Follows Interface Segregation Principle (ISP) - clients should not be forced
 * to depend on interfaces they do not use.
 */
class HasRandomizer
{
  public:
    virtual ~HasRandomizer() = default;
    virtual Randomizer& randomizer() = 0;
};

/**
 * @brief Simple wrapper around a Randomizer instance
 */
class RandomizerService : public HasRandomizer
{
  public:
    explicit RandomizerService(std::shared_ptr<Randomizer> randomizer) : m_randomizer(randomizer) {}
    
    Randomizer& randomizer() override { return *m_randomizer; }
    
  private:
    std::shared_ptr<Randomizer> m_randomizer;
};

} // namespace pkt::core