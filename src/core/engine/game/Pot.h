// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.


#pragma once

#include <list>
#include <memory>
#include <vector>
#include "core/player/Player.h"
#include "core/ports/Logger.h"

namespace pkt::core
{

/**
 * @brief Manages pot distribution and winner calculation in poker hands.
 * 
 * Handles the distribution of chips from the pot to winning players
 * at the end of a hand, including side pot calculations and proper
 * chip distribution among multiple winners.
 */
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
    void determineAbsoluteBestHands();

    std::shared_ptr<player::Player> resolveRemainderReceiver(const std::vector<size_t>& winnerIndices) const;
    std::vector<size_t> indexesOf(const std::list<unsigned>& ids);
    Logger& m_logger; 
};

} // namespace pkt::core
