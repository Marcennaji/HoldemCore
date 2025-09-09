#pragma once

#include <list>
#include <memory>
#include <vector>
#include "core/player/PlayerFsm.h"

namespace pkt::core
{

class PotFsm
{
  public:
    PotFsm(unsigned total, pkt::core::player::PlayerFsmList seats, unsigned dealerId);

    void distribute();
    const std::list<unsigned>& getWinners() const { return myWinners; }

  private:
    unsigned myTotal;
    pkt::core::player::PlayerFsmList mySeats;
    std::list<unsigned> myWinners;
    unsigned myDealerId;

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

    std::shared_ptr<player::PlayerFsm> resolveRemainderReceiver(const std::vector<size_t>& winnerIndices) const;
    std::vector<size_t> indexesOf(const std::list<unsigned>& ids);
};

} // namespace pkt::core
