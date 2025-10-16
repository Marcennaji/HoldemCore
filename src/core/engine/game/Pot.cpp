#include "Pot.h"
#include <algorithm>
#include <stdexcept>
#include "core/player/Helpers.h"

namespace pkt::core
{

Pot::Pot(unsigned total, pkt::core::player::PlayerList seats, unsigned dealerId, Logger& logger)
    : m_total(total), m_seats(std::move(seats)), m_dealerId(dealerId), m_logger(logger)
{
}

void Pot::distribute()
{
    m_winners.clear();
    std::vector<unsigned> contributions = initializePlayerContributions();
    std::vector<unsigned> remaining = contributions;
    std::sort(remaining.begin(), remaining.end());

    while (hasRemainingChips(remaining))
    {
        unsigned level = nextLevelAmount(remaining);
        // Eligible winners are non-folded players who have contributed at least "level".
        auto eligible = eligibleContributors(contributions, level);
        // Contributors to the pot at this level include ALL players (folded or not) whose
        // remaining contribution is at least "level". Their chips are in the pot even if
        // they cannot win it.
        size_t contributorsCount = 0;
        for (size_t idx = 0; idx < contributions.size(); ++idx)
        {
            if (contributions[idx] >= level)
                ++contributorsCount;
        }

        if (contributorsCount == 0)
            break;

        int potLevel = static_cast<int>(contributorsCount * level);
        if (potLevel > m_total)
        {
            m_logger.info("Pot level " + std::to_string(potLevel) +
                          " exceeds total available chips : " + std::to_string(m_total));
            potLevel = m_total;
        }

        // If no non-folded player is eligible to win this pot (e.g., only folders contributed
        // to this level), then there is no one to award; stop and let finalizeDistribution
        // split the remaining total among already-determined winners (or non-folded players as
        // a last resort). This prevents incorrectly fabricating a winner for this level.
        if (eligible.empty())
        {
            break;
        }

        auto winners = determineWinners(eligible, level);
        if (winners.empty())
            winners = eligible;

        int baseShare = potLevel / static_cast<int>(winners.size());
        int remainder = potLevel % static_cast<int>(winners.size());

        awardBaseShare(winners, baseShare);
        awardRemainder(winners, remainder);

        reduceContributions(contributions, level);
        remaining = contributions;
        std::sort(remaining.begin(), remaining.end());

        m_total -= potLevel;
    }

    finalizeDistribution();

    // After all pots distributed, determine who had the absolute best hand(s)
    // for GUI display (only show players with strongest hand, not all side pot winners)
    determineAbsoluteBestHands();
    // Note: determineAbsoluteBestHands() replaces m_winners with only the absolute best hands
    // The list is already properly formed, no need for sort/unique after
}

std::vector<unsigned> Pot::initializePlayerContributions()
{
    std::vector<unsigned> result;
    for (const auto& p : *m_seats)
    {
        unsigned contributed = p->getCashAtHandStart() - p->getCash();
        result.push_back(contributed);
        p->setLastMoneyWon(0);
    }
    return result;
}

bool Pot::hasRemainingChips(const std::vector<unsigned>& c) const
{
    return std::any_of(c.begin(), c.end(), [](unsigned x) { return x > 0; });
}

unsigned Pot::nextLevelAmount(const std::vector<unsigned>& c) const
{
    for (unsigned x : c)
        if (x > 0)
            return x;
    return 0;
}

std::vector<size_t> Pot::eligibleContributors(const std::vector<unsigned>& contributions, unsigned level) const
{
    std::vector<size_t> result;
    size_t index = 0;
    for (const auto& p : *m_seats)
    {
        if (p->getLastAction().type != ActionType::Fold && contributions[index] >= level)
            result.push_back(index);
        ++index;
    }
    return result;
}

std::vector<size_t> Pot::determineWinners(const std::vector<size_t>& eligible, unsigned level) const
{
    int bestRank = 0;
    for (size_t i : eligible)
    {
        auto it = std::next(m_seats->begin(), i);
        int rank = (*it)->getHandRanking();
        m_logger.verbose("  Player " + std::to_string((*it)->getId()) + " with hole cards " +
                         (*it)->getHoleCards().toString() + ", rank: " + std::to_string(rank));
        if (rank > bestRank)
            bestRank = rank;
    }
    m_logger.verbose("Best rank at this level: " + std::to_string(bestRank) + " (higher is better)");

    std::vector<size_t> winners;
    for (size_t i : eligible)
    {
        auto it = std::next(m_seats->begin(), i);
        if ((*it)->getHandRanking() == bestRank)
        {
            m_logger.verbose("  Winner: Player " + std::to_string((*it)->getId()) + " with rank " +
                             std::to_string(bestRank) + " corresponding to hole cards " +
                             (*it)->getHoleCards().toString());
            winners.push_back(i);
        }
    }

    return winners;
}

void Pot::awardBaseShare(const std::vector<size_t>& winners, int share)
{
    for (size_t i : winners)
    {
        auto it = std::next(m_seats->begin(), i);
        (*it)->setCash((*it)->getCash() + share);
        (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + share);
        m_winners.push_back((*it)->getId());
    }
}

void Pot::awardRemainder(const std::vector<size_t>& winners, int remainder)
{
    if (remainder == 0 || winners.empty())
        return;

    auto receiver = resolveRemainderReceiver(winners);
    if (receiver)
    {
        receiver->setCash(receiver->getCash() + remainder);
        receiver->setLastMoneyWon(receiver->getLastMoneyWon() + remainder);
    }
}

std::shared_ptr<player::Player> Pot::resolveRemainderReceiver(const std::vector<size_t>& winnerIndexes) const
{
    if (winnerIndexes.empty() || !m_seats || m_seats->empty())
        return nullptr;

    // Find the dealer in the seats list
    auto dealerIt = std::find_if(m_seats->begin(), m_seats->end(),
                                 [this](const std::shared_ptr<player::Player>& p) { return p->getId() == m_dealerId; });

    // If dealer not found, fallback to first winner
    if (dealerIt == m_seats->end())
        return *std::next(m_seats->begin(), winnerIndexes.front());

    // Start iterating clockwise from dealer
    auto it = dealerIt;
    for (size_t i = 0; i < m_seats->size(); ++i)
    {
        ++it;
        if (it == m_seats->end())
            it = m_seats->begin();

        size_t currentIndex = std::distance(m_seats->begin(), it);
        if (std::find(winnerIndexes.begin(), winnerIndexes.end(), currentIndex) != winnerIndexes.end())
        {
            return *it;
        }
    }

    // Fallback: just return first winner
    return *std::next(m_seats->begin(), winnerIndexes.front());
}

void Pot::reduceContributions(std::vector<unsigned>& c, unsigned level)
{
    for (unsigned& x : c)
        x = (x >= level) ? (x - level) : 0;
}

void Pot::finalizeDistribution()
{
    if (m_total == 0 || !m_seats || m_seats->empty())
        return;

    // Create a unique set of winner IDs
    std::list<unsigned> uniqueWinners = m_winners;
    uniqueWinners.sort();
    uniqueWinners.unique();

    // Fallback: if no winners, pick all non-folded players
    if (uniqueWinners.empty())
    {
        for (const auto& player : *m_seats)
        {
            if (player->getLastAction().type != ActionType::Fold)
                uniqueWinners.push_back(player->getId());
        }
        uniqueWinners.sort();
        uniqueWinners.unique();
    }

    if (uniqueWinners.empty())
        return;

    const int numWinners = static_cast<int>(uniqueWinners.size());
    const int baseShare = m_total / numWinners;
    int remainder = m_total % numWinners;

    // Distribute base share to each winner
    for (unsigned id : uniqueWinners)
    {
        auto it = getPlayerListIteratorById(m_seats, id);
        if (it != m_seats->end())
        {
            (*it)->setCash((*it)->getCash() + baseShare);
            (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + baseShare);
        }
    }

    // Distribute remainder to one or more winners (clockwise from dealer)
    if (remainder > 0)
    {
        auto receiver = resolveRemainderReceiver(indexesOf(uniqueWinners));
        if (receiver)
        {
            receiver->setCash(receiver->getCash() + remainder);
            receiver->setLastMoneyWon(receiver->getLastMoneyWon() + remainder);
        }
    }

    m_total = 0;
}

std::vector<size_t> Pot::indexesOf(const std::list<unsigned>& ids)
{
    std::vector<size_t> result;

    for (unsigned id : ids)
    {
        size_t index = 0;
        for (auto it = m_seats->begin(); it != m_seats->end(); ++it, ++index)
        {
            if ((*it)->getId() == id)
            {
                result.push_back(index);
                break;
            }
        }
    }

    return result;
}

void Pot::determineAbsoluteBestHands()
{
    if (!m_seats || m_seats->empty())
        return;

    // Find the absolute best hand among all non-folded players
    int bestRank = 0;

    m_logger.verbose("Determining absolute best hand for GUI display:");

    for (const auto& player : *m_seats)
    {
        if (!player)
            continue;

        if (player->getLastAction().type != ActionType::Fold)
        {
            int rank = player->getHandRanking();
            m_logger.verbose("  Player " + std::to_string(player->getId()) +
                             " (non-folded) has rank: " + std::to_string(rank));
            if (rank > bestRank)
            {
                bestRank = rank;
            }
        }
    }

    m_logger.verbose("Best rank overall: " + std::to_string(bestRank));

    // If bestRank is still 0, it means all players folded or have no hand ranking
    // In this case, keep the current winners list (from pot distribution)
    if (bestRank == 0)
    {
        m_logger.verbose("No valid hand rankings found, keeping current winners");
        return;
    }

    // Clear and replace m_winners with only players who have the absolute best hand
    m_winners.clear();

    // Add all players who have the best rank (handles ties)
    for (const auto& player : *m_seats)
    {
        if (!player)
            continue;

        if (player->getLastAction().type != ActionType::Fold && player->getHandRanking() == bestRank)
        {
            m_winners.push_back(player->getId());
            m_logger.verbose("  GUI Winner: Player " + std::to_string(player->getId()) + " with best rank " +
                             std::to_string(bestRank));
        }
    }
}

} // namespace pkt::core
