// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PositionManager.h"
#include "core/player/Player.h"
#include <vector>

using namespace pkt::core::player;

namespace pkt::core::player::position
{

bool PositionManager::hasPosition(PlayerPosition position, const PlayerList& actingPlayers)
{
    // return true if position is last to play, false if not
    bool hasPositionalAdvantage = true;

    for (auto itC = actingPlayers->begin(); itC != actingPlayers->end(); ++itC)
    {
        if ((*itC)->getPosition() > position)
        {
            hasPositionalAdvantage = false;
            break;
        }
    }

    return hasPositionalAdvantage;
}

int PositionManager::playerDistanceCircularOffset(int fromId, int toId, const PlayerList& players)
{
    // Compute index of fromId (dealer) and toId in the seating order
    int fromIndex = -1;
    int toIndex = -1;
    int idx = 0;
    for (auto& p : *players)
    {
        if (p->getId() == fromId)
            fromIndex = idx;
        if (p->getId() == toId)
            toIndex = idx;
        ++idx;
    }

    // Fallback if not found (shouldn't happen in valid games)
    if (fromIndex < 0 || toIndex < 0)
        return 0;

    // Distance moving clockwise (increasing indices) from fromIndex to toIndex with wrap-around
    const int n = idx; // number of players
    int distance = toIndex - fromIndex;
    if (distance < 0)
        distance += n;
    return distance;
}

PlayerPosition PositionManager::computePlayerPositionFromOffset(int offset, int nbPlayers)
{
    // Define position mappings for each table size
    static const std::vector<std::vector<PlayerPosition>> positionMaps = {
        {}, // 0 players (unused)
        {}, // 1 player (unused)
        {ButtonSmallBlind, BigBlind}, // 2 players
        {Button, SmallBlind, BigBlind}, // 3 players
        {Button, SmallBlind, BigBlind, UnderTheGun}, // 4 players
        {Button, SmallBlind, BigBlind, UnderTheGun, Cutoff}, // 5 players
        {Button, SmallBlind, BigBlind, UnderTheGun, Middle, Cutoff}, // 6 players
        {Button, SmallBlind, BigBlind, UnderTheGun, Middle, Cutoff, Late}, // 7 players
        {Button, SmallBlind, BigBlind, UnderTheGun, UnderTheGunPlusOne, Middle, Cutoff, Late}, // 8 players
        {Button, SmallBlind, BigBlind, UnderTheGun, UnderTheGunPlusOne, UnderTheGunPlusTwo, Middle, Cutoff, Late}, // 9 players
        {Button, SmallBlind, BigBlind, UnderTheGun, UnderTheGunPlusOne, UnderTheGunPlusTwo, Middle, MiddlePlusOne, Cutoff, Late} // 10 players
    };

    // Handle specific table sizes with predefined mappings
    if (nbPlayers >= 2 && nbPlayers <= 10 && offset >= 0 && offset < nbPlayers)
    {
        return positionMaps[nbPlayers][offset];
    }

    // Fallback
    return Unknown;
}

} // namespace pkt::core::player::position
