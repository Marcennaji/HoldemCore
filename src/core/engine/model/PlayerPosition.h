// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
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

} // namespace pkt::core
