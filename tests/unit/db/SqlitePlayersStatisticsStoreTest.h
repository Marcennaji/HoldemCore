
#pragma once

#include "common/EngineTest.h"
#include "infra/persistence/SqlitePlayersStatisticsStore.h"

#include <memory>

namespace pkt::test
{

class SqlitePlayersStatisticsStoreTest : public EngineTest
{
  public:
    void SetUp() override;
    void TearDown() override;
};
} // namespace pkt::test