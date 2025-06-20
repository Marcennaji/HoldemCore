// File: core/services/GlobalServices.hpp

#pragma once

#include "core/interfaces/ILogger.h"
#include "core/interfaces/persistence/IHandAuditStore.h"
#include "core/interfaces/persistence/IPlayersStatisticsStore.h"
#include "core/interfaces/persistence/IRankingStore.h"

#include <memory>

namespace pkt::core
{

class GlobalServices
{
  public:
    static GlobalServices& instance()
    {
        static GlobalServices inst;
        return inst;
    }

    // Getters
    std::shared_ptr<ILogger> logger() const { return myLogger; }
    std::shared_ptr<IRankingStore> rankingStore() const { return myRankingStore; }
    std::shared_ptr<IHandAuditStore> handAuditStore() const { return myHandAuditStore; }
    std::shared_ptr<IPlayersStatisticsStore> playersStatisticsStore() const { return myPlayersStatisticsStore; }

    // Setters
    void setLogger(std::shared_ptr<ILogger> logger) { myLogger = std::move(logger); }
    void setRankingStore(std::shared_ptr<IRankingStore> store) { myRankingStore = std::move(store); }
    void setHandAuditStore(std::shared_ptr<IHandAuditStore> store) { myHandAuditStore = std::move(store); }
    void setPlayersStatisticsStore(std::shared_ptr<IPlayersStatisticsStore> store)
    {
        myPlayersStatisticsStore = std::move(store);
    }

  private:
    GlobalServices();

    std::shared_ptr<ILogger> myLogger;
    std::shared_ptr<IRankingStore> myRankingStore;
    std::shared_ptr<IHandAuditStore> myHandAuditStore;
    std::shared_ptr<IPlayersStatisticsStore> myPlayersStatisticsStore;
};

} // namespace pkt::core
