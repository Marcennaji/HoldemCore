// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <list>
#include <memory>

namespace pkt::core::player
{

class PlayerFsm;

typedef std::shared_ptr<std::list<std::shared_ptr<PlayerFsm>>> PlayerFsmList;
typedef std::list<std::shared_ptr<PlayerFsm>>::iterator PlayerFsmListIterator;
typedef std::list<std::shared_ptr<PlayerFsm>>::const_iterator PlayerFsmListConstIterator;
} // namespace pkt::core::player
