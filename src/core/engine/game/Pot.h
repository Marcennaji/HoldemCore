#pragma once

#include <list>
#include <memory>
#include <vector>
#include "core/player/Player.h"
#include "core/interfaces/Logger.h"

namespace pkt::core
{

class Pot
{
  public:

    Pot(unsigned total, pkt::core::player::PlayerList seats, unsigned dealerId,
        Logger& logger);

    void distribute();
    const std::list<unsigned>& getWinners() const { return m_winners; }

  private:
    unsigned m_total;
    pkt::core::player::PlayerList m_seats;
    std::list<unsigned> m_winners;
    unsigned m_dealerId;

    // Distribution steps
    std::vector<unsigned> initializePlayerContributions();
    bool hasRemainingChips(const std::vector<unsigned>& contributions) const;
    unsigned nextLevelAmount(const std::vector<unsigned>& contributions) const;
    std::vector<size_t> eligibleContributors(const std::vector<unsigned>& contributions, unsigned level) const;
    std::vector<size_t> determineWinners(const std::vector<size_t>& eligible, unsigned level) const;
    void awardBaseShare(const std::vector<size_t>& winnerIndices, int share);
    void awardRemainder(const std::vector<size_t>& winnerIndices, int remainder);
    void reduceContributions(std::vector<unsigned>& contributions, unsigned level);
    void finalizeDistribution();

    std::shared_ptr<player::Player> resolveRemainderReceiver(const std::vector<size_t>& winnerIndices) const;
    std::vector<size_t> indexesOf(const std::list<unsigned>& ids);
    Logger* m_logger; 
};

} // namespace pkt::core
