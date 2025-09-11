
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

namespace pkt::core::player
{
class PlayerFsm;
} // namespace pkt::core::player

namespace pkt::core
{

class BettingActions;
class HandFsm;
class IHandState;
struct GameEvents;
struct PlayerAction;

std::shared_ptr<player::PlayerFsm> getFirstPlayerToActPostFlop(const HandFsm& hand);
std::unique_ptr<pkt::core::IHandState> computeBettingRoundNextState(HandFsm& hand, const GameEvents& events,
                                                                    GameState currentState);
bool isRoundComplete(HandFsm& hand);
std::vector<ActionType> getValidActionsForPlayer(const HandFsm& hand, int playerId);
bool validatePlayerAction(const pkt::core::player::PlayerFsmList& actingPlayersList, const PlayerAction& action,
                          const BettingActions& bettingActions, int smallBlind, const GameState gameState);

} // namespace pkt::core