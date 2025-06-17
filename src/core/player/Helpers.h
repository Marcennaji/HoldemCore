
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <iostream>
#include <list>
#include <memory>
#include <string>

struct PostFlopState;

namespace pkt::core::player
{
class Player;

void shufflePlayers(std::list<std::shared_ptr<Player>>& players, unsigned humanId = 0);
bool isCardsInRange(std::string card1, std::string card2, std::string range);
std::string getFakeCard(char c);
const int getDrawingProbability(const PostFlopState& state);
bool isDrawingProbOk(const PostFlopState&, const int potOdd);
const int getImplicitOdd(const PostFlopState& state);
int getBoardCardsHigherThan(std::string stringBoard, std::string card);

} // namespace pkt::core::player