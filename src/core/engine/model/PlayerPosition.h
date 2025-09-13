// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <string>
namespace pkt::core
{

enum PlayerPosition
{
    Unknown = 0,
    SmallBlind = 1,
    BigBlind = 2,
    UnderTheGun = 3,
    UnderTheGunPlusOne = 4,
    UnderTheGunPlusTwo = 5,
    Middle = 6,
    MiddlePlusOne = 7,
    Late = 8,
    Cutoff = 9,
    Button = 10,
    ButtonSmallBlind = 11
};

static std::string positionToString(PlayerPosition p)
{

    switch (p)
    {

    case UnderTheGun:
        return "UnderTheGun";
        break;
    case UnderTheGunPlusOne:
        return "UnderTheGun+1";
        break;
    case UnderTheGunPlusTwo:
        return "UnderTheGun+2";
        break;
    case Middle:
        return "Middle";
        break;
    case MiddlePlusOne:
        return "Middle+1";
        break;
    case Late:
        return "Late";
        break;
    case Cutoff:
        return "Cutoff";
        break;
    case Button:
        return "Button";
        break;
    case ButtonSmallBlind:
        return "Button / Small Blind";
        break;
    case SmallBlind:
        return "Small Blind";
        break;
    case BigBlind:
        return "Big Blind";
        break;
    default:
        return "unknown";
        break;
    }
}

} // namespace pkt::core
