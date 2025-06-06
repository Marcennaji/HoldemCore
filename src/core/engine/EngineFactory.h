/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#pragma once

#include "EngineFactory.h"

#include <core/interfaces/ILogger.h>
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
    EngineFactory(GameEvents*, ILogger*);
    ~EngineFactory();

    virtual std::shared_ptr<IHand> createHand(std::shared_ptr<EngineFactory> f, std::shared_ptr<IBoard> b,
                                              IRankingStore* l, IPlayersStatisticsStore* ps, IHandAuditStore* hs,
                                              pkt::core::player::PlayerList sl, pkt::core::player::PlayerList apl,
                                              pkt::core::player::PlayerList rpl, int id, int sP, int dP, int sB,
                                              int sC);
    virtual std::shared_ptr<IBoard> createBoard(unsigned dp);
    virtual std::vector<std::shared_ptr<IBettingRound>> createBettingRound(IHand* hi, unsigned dP, int sB);

  private:
    ILogger* myLogger;
    GameEvents* myEvents;
};

} // namespace pkt::core
