
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
    // Override to use Info level logging for this test
    pkt::core::LogLevel getTestLogLevel() const override { return pkt::core::LogLevel::Info; }
    
    std::shared_ptr<pkt::core::AppServiceContainer> m_testServices;
};
} // namespace pkt::test