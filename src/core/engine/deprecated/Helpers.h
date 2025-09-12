
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/model/PlayerPosition.h"
#include "core/player/typedefs.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace pkt::core
{

pkt::core::player::PlayerListIterator findPlayerOrThrow(pkt::core::player::PlayerList seats, unsigned id);
pkt::core::player::PlayerListIterator nextActivePlayer(pkt::core::player::PlayerList seats,
                                                       pkt::core::player::PlayerListIterator it);
void updateActingPlayersList(pkt::core::player::PlayerList&);
std::string positionToString(pkt::core::PlayerPosition);
std::shared_ptr<pkt::core::player::Player> getPlayerById(pkt::core::player::PlayerList list, unsigned id);
pkt::core::player::PlayerListIterator getPlayerListIteratorById(pkt::core::player::PlayerList list, unsigned id);
void shufflePlayers(std::list<std::shared_ptr<pkt::core::player::Player>>& players, unsigned humanId);
} // namespace pkt::core