// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.



#pragma once

#include "common/EngineTest.h"
#include "adapters/infrastructure/statistics/sqlite/SqlitePlayersStatisticsStore.h"

#include <memory>

namespace pkt::test
{

class SqlitePlayersStatisticsStoreTest : public EngineTest
{
  public:
    void SetUp() override;
    void TearDown() override;

  protected:
    pkt::core::LogLevel getTestLogLevel() const override { return pkt::core::LogLevel::Info; }
};
} // namespace pkt::test