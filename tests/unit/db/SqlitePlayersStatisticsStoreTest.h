
#pragma once

#include "common/EngineTest.h"
#include "infra/persistence/SqlitePlayersStatisticsStore.h"

#include <memory>

namespace pkt::core
{
class AppServiceContainer;
}

namespace pkt::test
{

class SqlitePlayersStatisticsStoreTest : public EngineTest
{
  public:
    void SetUp() override;
    void TearDown() override;

  protected:
    std::shared_ptr<pkt::core::AppServiceContainer> myTestServices;
};
} // namespace pkt::test