
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/model/PlayerPosition.h"
#include "typedefs.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>

struct PostFlopAnalysisFlags;

namespace pkt::core
{
class BettingActions;
class HandFsm;
struct PlayerAction;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;

void shufflePlayers(std::list<std::shared_ptr<Player>>& players, unsigned humanId = 0);
bool isCardsInRange(std::string card1, std::string card2, std::string range);
std::string getFakeCard(char c);
const int getDrawingProbability(const PostFlopAnalysisFlags& state);
bool isDrawingProbOk(const PostFlopAnalysisFlags&, const int potOdd);
const int getImplicitOdd(const PostFlopAnalysisFlags& state);
int getBoardCardsHigherThan(std::string stringBoard, std::string card);
// Ensures the first card is the highest based on card ordering
void ensureHighestCard(std::string& card1, std::string& card2);

// Validates if the given range token is valid
bool isValidRange(const std::string& token);

// Checks if the cards match an exact pair range
bool isExactPair(const char* c1, const char* c2, const char* range);

// Checks if the cards match an exact suited hand range
bool isExactSuitedHand(const char* c1, const char* c2, const char* range);

// Checks if the cards match an exact offsuited hand range
bool isExactOffsuitedHand(const char* c1, const char* c2, const char* range);

// Checks if the cards match a pair and above range
bool isPairAndAbove(const std::string& card1, const std::string& card2, const char* range);

// Checks if the cards match an offsuited and above range
bool isOffsuitedAndAbove(const std::string& card1, const std::string& card2, const char* c1, const char* c2,
                         const char* range);

// Checks if the cards match a suited and above range
bool isSuitedAndAbove(const std::string& card1, const std::string& card2, const char* c1, const char* c2,
                      const char* range);

// Checks if the cards match an exact hand range
bool isExactHand(const std::string& card1, const std::string& card2, const char* range);

std::shared_ptr<Player> getPlayerById(PlayerList list, unsigned id);
PlayerListIterator getPlayerListIteratorById(PlayerList list, unsigned id);

std::shared_ptr<PlayerFsm> getPlayerFsmById(PlayerFsmList list, unsigned id);
PlayerFsmListIterator getPlayerFsmListIteratorById(PlayerFsmList list, unsigned id);

PlayerListIterator findPlayerOrThrow(PlayerList seats, unsigned id);
PlayerListIterator nextActivePlayer(PlayerList seats, PlayerListIterator it);

void updateRunningPlayersList(PlayerList&);
void updateRunningPlayersListFsm(PlayerFsmList&);

std::string getPositionLabel(pkt::core::PlayerPosition);

bool hasPosition(PlayerPosition myPos, PlayerFsmList);

bool validatePlayerAction(const PlayerFsm& player, const PlayerAction& action, const BettingActions& bettingActions,
                          int smallBlind);

int circularOffset(int fromId, int toId, const PlayerFsmList& players);
PlayerPosition computePositionFromOffset(int offset, int nbPlayers);

bool isDealerPosition(PlayerPosition p);

bool isSmallBlindPosition(PlayerPosition p);

bool isRoundComplete(const HandFsm& hand);

} // namespace pkt::core::player