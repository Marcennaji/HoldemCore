
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
std::string getStringRange(int nbPlayers, int range);
const int getDrawingProbability(const PostFlopState& state);
bool isDrawingProbOk(const PostFlopState&, const int potOdd);
const int getImplicitOdd(const PostFlopState& state);
int getBoardCardsHigherThan(std::string stringBoard, std::string card);
void logPostFlopState(const Player& player, const PostFlopState& state, std::ostream& out = std::cout);

} // namespace pkt::core::player