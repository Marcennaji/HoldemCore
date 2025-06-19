// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "HumanPlayer.h"
#include <core/player/strategy/CurrentHandContext.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core::player
{

using namespace std;

HumanPlayer::HumanPlayer(const GameEvents& events, ILogger* logger, IHandAuditStore* ha, IPlayersStatisticsStore* ps,
                         int id, std::string name, int sC, bool aS, int mB)
    : Player(events, logger, ha, ps, id, name, sC, aS, mB)
{
}

HumanPlayer::~HumanPlayer()
{
}

} // namespace pkt::core::player
