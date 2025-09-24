// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PositionManager.h"
#include "core/player/Player.h"

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
    // Dealer always at 0
    switch (nbPlayers)
    {
    case 2:
        return offset == 0 ? ButtonSmallBlind : BigBlind;
    case 3:
        return (offset == 0) ? Button : (offset == 1 ? SmallBlind : BigBlind);
    case 4:
        return (offset == 0) ? Button : (offset == 1 ? SmallBlind : (offset == 2 ? BigBlind : UnderTheGun));
    case 5:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        return Cutoff;
    case 6:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return Middle;
        return Cutoff;
    case 7:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return Middle;
        if (offset == 5)
            return Cutoff;
        return Late;
    case 8:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return UnderTheGunPlusOne;
        if (offset == 5)
            return Middle;
        if (offset == 6)
            return Cutoff;
        return Late;
    case 9:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return UnderTheGunPlusOne;
        if (offset == 5)
            return UnderTheGunPlusTwo;
        if (offset == 6)
            return Middle;
        if (offset == 7)
            return Cutoff;
        return Late;
    default:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset <= 5)
            return UnderTheGunPlusOne;
        if (offset <= 7)
            return Middle;
        if (offset == nbPlayers - 2)
            return Cutoff;
        return Late;
    }
}

} // namespace pkt::core::player::position
