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
    std::shared_ptr<ILogger> logger() const { return logger_; }
    std::shared_ptr<IRankingStore> rankingStore() const { return rankingStore_; }
    std::shared_ptr<IHandAuditStore> handAuditStore() const { return handAuditStore_; }
    std::shared_ptr<IPlayersStatisticsStore> playersStatisticsStore() const { return playersStatisticsStore_; }

    // Setters
    void setLogger(std::shared_ptr<ILogger> logger) { logger_ = std::move(logger); }
    void setRankingStore(std::shared_ptr<IRankingStore> store) { rankingStore_ = std::move(store); }
    void setHandAuditStore(std::shared_ptr<IHandAuditStore> store) { handAuditStore_ = std::move(store); }
    void setPlayersStatisticsStore(std::shared_ptr<IPlayersStatisticsStore> store)
    {
        playersStatisticsStore_ = std::move(store);
    }

  private:
    GlobalServices();

    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IRankingStore> rankingStore_;
    std::shared_ptr<IHandAuditStore> handAuditStore_;
    std::shared_ptr<IPlayersStatisticsStore> playersStatisticsStore_;
};

} // namespace pkt::core
