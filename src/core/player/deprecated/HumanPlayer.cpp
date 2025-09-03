// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "core/player/deprecated/HumanPlayer.h"
#include <core/player/strategy/CurrentHandContext.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core::player
{

using namespace std;

HumanPlayer::HumanPlayer(const GameEvents& events, int id, int sC, bool aS, ButtonState mB)
    : Player(events, id, HumanPlayer::getName(), sC, aS, mB)
{
}

HumanPlayer::~HumanPlayer() = default;

} // namespace pkt::core::player
