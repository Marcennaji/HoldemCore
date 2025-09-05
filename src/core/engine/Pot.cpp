#include "Pot.h"
#include <algorithm>
#include <stdexcept>
#include "core/engine/Exception.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{

Pot::Pot(unsigned total, pkt::core::player::PlayerList seats, unsigned dealerId)
    : myTotal(total), mySeats(std::move(seats)), myDealerId(dealerId)
{
}

void Pot::distribute()
{
    myWinners.clear();
    std::vector<unsigned> contributions = initializePlayerContributions();
    std::vector<unsigned> remaining = contributions;
    std::sort(remaining.begin(), remaining.end());

    while (hasRemainingChips(remaining))
    {
        unsigned level = nextLevelAmount(remaining);
        auto eligible = eligibleContributors(contributions, level);

        if (eligible.empty())
            break;

        int potLevel = static_cast<int>(eligible.size() * level);
        if (potLevel > myTotal)
        {
            GlobalServices::instance().logger().info("Pot level " + std::to_string(potLevel) +
                                                     " exceeds total available chips : " + std::to_string(myTotal));
            potLevel = myTotal;
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

        myTotal -= potLevel;
    }

    finalizeDistribution();

    myWinners.sort();
    myWinners.unique();
}

std::vector<unsigned> Pot::initializePlayerContributions()
{
    std::vector<unsigned> result;
    for (const auto& p : *mySeats)
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
    for (const auto& p : *mySeats)
    {
        if (p->getAction().type != ActionType::Fold && contributions[index] >= level)
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
        auto it = std::next(mySeats->begin(), i);
        if ((*it)->getHandRanking() > bestRank)
            bestRank = (*it)->getHandRanking();
    }

    std::vector<size_t> winners;
    for (size_t i : eligible)
    {
        auto it = std::next(mySeats->begin(), i);
        if ((*it)->getHandRanking() == bestRank)
            winners.push_back(i);
    }

    return winners;
}

void Pot::awardBaseShare(const std::vector<size_t>& winners, int share)
{
    for (size_t i : winners)
    {
        auto it = std::next(mySeats->begin(), i);
        (*it)->setCash((*it)->getCash() + share);
        (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + share);
        myWinners.push_back((*it)->getId());
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
    if (winnerIndexes.empty() || !mySeats || mySeats->empty())
        return nullptr;

    // Find the dealer in the seats list
    auto dealerIt = std::find_if(mySeats->begin(), mySeats->end(),
                                 [this](const std::shared_ptr<player::Player>& p) { return p->getId() == myDealerId; });

    // If dealer not found, fallback to first winner
    if (dealerIt == mySeats->end())
        return *std::next(mySeats->begin(), winnerIndexes.front());

    // Start iterating clockwise from dealer
    auto it = dealerIt;
    for (size_t i = 0; i < mySeats->size(); ++i)
    {
        ++it;
        if (it == mySeats->end())
            it = mySeats->begin();

        size_t currentIndex = std::distance(mySeats->begin(), it);
        if (std::find(winnerIndexes.begin(), winnerIndexes.end(), currentIndex) != winnerIndexes.end())
        {
            return *it;
        }
    }

    // Fallback: just return first winner
    return *std::next(mySeats->begin(), winnerIndexes.front());
}

void Pot::reduceContributions(std::vector<unsigned>& c, unsigned level)
{
    for (unsigned& x : c)
        x = (x >= level) ? (x - level) : 0;
}

void Pot::finalizeDistribution()
{
    if (myTotal == 0 || !mySeats || mySeats->empty())
        return;

    // Create a unique set of winner IDs
    std::list<unsigned> uniqueWinners = myWinners;
    uniqueWinners.sort();
    uniqueWinners.unique();

    // Fallback: if no winners, pick all non-folded players
    if (uniqueWinners.empty())
    {
        for (const auto& player : *mySeats)
        {
            if (player->getAction().type != ActionType::Fold)
                uniqueWinners.push_back(player->getId());
        }
        uniqueWinners.sort();
        uniqueWinners.unique();
    }

    if (uniqueWinners.empty())
        return;

    const int numWinners = static_cast<int>(uniqueWinners.size());
    const int baseShare = myTotal / numWinners;
    int remainder = myTotal % numWinners;

    // Distribute base share to each winner
    for (unsigned id : uniqueWinners)
    {
        auto it = getPlayerListIteratorById(mySeats, id);
        if (it != mySeats->end())
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

    myTotal = 0;
}

std::vector<size_t> Pot::indexesOf(const std::list<unsigned>& ids)
{
    std::vector<size_t> result;

    for (unsigned id : ids)
    {
        size_t index = 0;
        for (auto it = mySeats->begin(); it != mySeats->end(); ++it, ++index)
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

} // namespace pkt::core
