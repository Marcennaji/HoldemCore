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
    Pot(unsigned total, pkt::core::player::PlayerList seats, unsigned dealerId);
    explicit Pot(unsigned total, pkt::core::player::PlayerList seats, unsigned dealerId,
                 std::shared_ptr<ServiceContainer> serviceContainer);
    
    // ISP-compliant constructor with focused services (preferred)
    Pot(unsigned total, pkt::core::player::PlayerList seats, unsigned dealerId,
        std::shared_ptr<Logger> logger);

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
    void ensureServicesInitialized() const;

    mutable std::shared_ptr<ServiceContainer> m_services; // Legacy service container (to be removed)
    std::shared_ptr<Logger> m_logger; // ISP-compliant focused service
};

} // namespace pkt::core
