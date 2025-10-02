
// HoldemCore — Texas Hold'em simulator
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

namespace pkt::core
{
class ServiceContainer;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;
} // namespace pkt::core::player

namespace pkt::core
{

class ActionValidator;
class BettingActions;
class Hand;
class HandState;
struct GameEvents;
struct PlayerAction;

std::shared_ptr<player::Player> getFirstPlayerToActPostFlop(const Hand& hand);
std::shared_ptr<player::Player> getNextPlayerToActInRound(const Hand& hand, GameState currentRound);
std::unique_ptr<pkt::core::HandState> computeBettingRoundNextState(Hand& hand, const GameEvents& events,
                                                                    GameState currentState);
bool isRoundComplete(const Hand& hand);
bool isRoundComplete(const Hand& hand, std::shared_ptr<pkt::core::ServiceContainer> services);
std::vector<ActionType> getValidActionsForPlayer(const Hand& hand, int playerId);
std::vector<ActionType> getValidActionsForPlayer(const pkt::core::player::PlayerList& actingPlayersList, int playerId,
                                                 const BettingActions& bettingActions, int smallBlind,
                                                 const GameState gameState);

// Legacy function for backwards compatibility - delegates to ActionValidator
bool validatePlayerAction(const pkt::core::player::PlayerList& actingPlayersList, const PlayerAction& action,
                          const BettingActions& bettingActions, int smallBlind, const GameState gameState);

} // namespace pkt::core