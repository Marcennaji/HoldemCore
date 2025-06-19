// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once
#include <string>

#include <core/interfaces/persistence/IHandAuditStore.h>

namespace pkt::core
{

class NullHandAuditStore : public IHandAuditStore
{
  public:
    virtual ~NullHandAuditStore() = default;

    virtual void updateUnplausibleHand(const std::string card1, const std::string card2, const bool human,
                                       const char bettingRound, const int nbPlayers) override
    {
    }
};
} // namespace pkt::core
