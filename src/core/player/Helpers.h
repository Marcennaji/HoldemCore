
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

namespace pkt::core
{
class BettingActions;
class HandFsm;
class IHandState;
struct GameEvents;
struct PlayerAction;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;

void shufflePlayers(std::list<std::shared_ptr<Player>>& players, unsigned humanId = 0);
bool isCardsInRange(std::string card1, std::string card2, std::string range);
const int getDrawingProbability(const PostFlopAnalysisFlags& state);
bool isDrawingProbOk(const PostFlopAnalysisFlags&, const int potOdd);
const int getImplicitOdd(const PostFlopAnalysisFlags& state);
int getBoardCardsHigherThan(std::string stringBoard, std::string card);

std::shared_ptr<Player> getPlayerById(PlayerList list, unsigned id);
PlayerListIterator getPlayerListIteratorById(PlayerList list, unsigned id);

std::shared_ptr<PlayerFsm> getPlayerFsmById(PlayerFsmList list, unsigned id);
PlayerFsmListIterator getPlayerFsmListIteratorById(PlayerFsmList list, unsigned id);

PlayerListIterator findPlayerOrThrow(PlayerList seats, unsigned id);
PlayerListIterator nextActivePlayer(PlayerList seats, PlayerListIterator it);
PlayerFsmListIterator nextActivePlayerFsm(PlayerFsmList seats, PlayerFsmListIterator it);

std::shared_ptr<player::PlayerFsm> getFirstPlayerToActPostFlop(const HandFsm& hand);

void updateActingPlayersList(PlayerList&);
void updateActingPlayersListFsm(PlayerFsmList&);

std::string getPositionLabel(pkt::core::PlayerPosition);

bool hasPosition(PlayerPosition myPos, PlayerFsmList);

std::vector<ActionType> getValidActionsForPlayer(const HandFsm& hand, int playerId);

std::vector<ActionType> getValidActionsForPlayer(const PlayerFsmList& actingPlayersList, int playerId,
                                                 const BettingActions& bettingActions, int smallBlind,
                                                 const GameState gameState);

bool validatePlayerAction(const PlayerFsmList& actingPlayersList, const PlayerAction& action,
                          const BettingActions& bettingActions, int smallBlind, const GameState gameState);

int circularOffset(int fromId, int toId, const PlayerFsmList& players);
PlayerPosition computePositionFromOffset(int offset, int nbPlayers);

bool isRoundComplete(HandFsm& hand);

std::unique_ptr<pkt::core::IHandState> computeBettingRoundNextState(HandFsm& hand, const GameEvents& events,
                                                                    GameState currentState);

} // namespace pkt::core::player