#pragma once
#include <string>

#include <core/engine/PlayerStatistics.h>

class IPlayerStatisticsStore {
public:
    virtual ~IPlayerStatisticsStore() = default;

    virtual void recordAction(const std::string& playerName, int actionType) = 0;
    virtual PlayerStatistics getStatistics(const std::string& playerName) const = 0;
};
