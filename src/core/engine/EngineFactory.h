// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "EngineFactory.h"

#include "core/interfaces/IBoard.h"
#include "core/interfaces/IHand.h"

#include <core/player/Player.h>

#include <memory>
#include <vector>

#include "core/player/typedefs.h"

namespace pkt::core
{
class EngineFactory
{
  public:
    EngineFactory(const GameEvents&);
    ~EngineFactory();

    virtual std::shared_ptr<IHand> createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                              pkt::core::player::PlayerList sl, pkt::core::player::PlayerList apl,
                                              pkt::core::player::PlayerList rpl, int id, int sP, int dP, int sB,
                                              int sC);
    virtual std::shared_ptr<IBoard> createBoard(unsigned dp);
    virtual std::vector<std::shared_ptr<IBettingRound>> createBettingRound(IHand* hi, unsigned dP, int sB);

  private:
    const GameEvents& myEvents;
};

} // namespace pkt::core
