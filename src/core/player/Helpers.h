
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "typedefs.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{
class Player;

std::shared_ptr<PlayerFsm> getPlayerFsmById(PlayerFsmList list, unsigned id);
PlayerFsmListIterator getPlayerFsmListIteratorById(PlayerFsmList list, unsigned id);
void updateActingPlayersListFsm(PlayerFsmList&);
bool isCardsInRange(std::string card1, std::string card2, std::string range);
const int getDrawingProbability(const PostFlopAnalysisFlags& state);
bool isDrawingProbOk(const PostFlopAnalysisFlags&, const int potOdd);
int getBoardCardsHigherThan(std::string stringBoard, std::string card);
bool hasPosition(PlayerPosition myPos, PlayerFsmList);
int circularOffset(int fromId, int toId, const PlayerFsmList& players);
PlayerPosition computePositionFromOffset(int offset, int nbPlayers);

} // namespace pkt::core::player