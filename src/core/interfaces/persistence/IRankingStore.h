#pragma once
#include <string>
#include <vector>

struct PlayerRanking {
    std::string playerName;
    int efficiencyScore;
};

class IRankingStore {
public:
    virtual ~IRankingStore() = default;

    virtual void updateRanking(const std::string& playerName, int score) = 0;
    virtual std::vector<PlayerRanking> loadRankings() const = 0;
};
