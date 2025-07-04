// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Board.h"

#include "core/interfaces/IHand.h"

#include <core/services/GlobalServices.h>
#include "Exception.h"
#include "Player.h"
#include "model/EngineError.h"

#include <algorithm>

namespace pkt::core
{
using namespace pkt::core::player;

Board::Board(unsigned dp) : IBoard(), myDealerPlayerId(dp)
{
    myCards[0] = myCards[1] = myCards[2] = myCards[3] = myCards[4] = 0;
}

Board::~Board()
{
    if (mySeatsList)
    {
        mySeatsList->clear();
    }
    if (myRunningPlayersList)
    {
        myRunningPlayersList->clear();
    }
}
void Board::setSeatsList(PlayerList seats)
{
    mySeatsList = seats;
}
void Board::setRunningPlayersList(PlayerList runningPlayers)
{
    myRunningPlayersList = runningPlayers;
}

void Board::collectSets()
{

    mySets = 0;

    PlayerListConstIterator itC;
    for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    {
        mySets += (*itC)->getSet();
    }
}

void Board::collectPot()
{

    myPot += mySets;
    mySets = 0;

    PlayerListIterator it;
    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->setSetNull();
    }
}

void Board::distributePot()
{
    myWinners.clear();
    std::vector<unsigned> contributions = distributePotInitializePlayerContributions();
    std::vector<unsigned> remainingContributions = contributions;
    std::sort(remainingContributions.begin(), remainingContributions.end());

    while (distributePotHasRemaining(remainingContributions))
    {
        unsigned levelAmount = distributePotNextLevelAmount(remainingContributions);
        std::vector<size_t> levelEligible = distributePotEligibleContributors(contributions, levelAmount);

        if (levelEligible.empty())
        {
            // No eligible players left at this level — exit early
            break;
        }

        int levelPot = static_cast<int>(levelEligible.size() * levelAmount);
        std::vector<size_t> levelWinners = distributePotDetermineWinners(levelEligible, levelAmount);

        if (levelWinners.empty())
        {
            // No winners — skip distribution for this level (but reduce contributions anyway)
            distributePotReduceContributions(contributions, levelAmount);
            myPot -= levelPot;
            continue;
        }

        int baseShare = levelPot / static_cast<int>(levelWinners.size());
        int remainder = levelPot % static_cast<int>(levelWinners.size());

        distributePotAwardBase(levelWinners, baseShare);
        distributePotDistributeRemainder(levelWinners, remainder);

        distributePotReduceContributions(contributions, levelAmount);
        myPot -= levelPot;
    }

    distributePotFinalize();
}
std::vector<unsigned> Board::distributePotInitializePlayerContributions()
{
    std::vector<unsigned> contributions;
    for (const auto& player : *mySeatsList)
    {
        unsigned contributed = player->getRoundStartCash() - player->getCash();
        contributions.push_back(contributed);
        player->setLastMoneyWon(0);
    }
    return contributions;
}

bool Board::distributePotHasRemaining(const std::vector<unsigned>& contributions) const
{
    return std::any_of(contributions.begin(), contributions.end(), [](unsigned c) { return c > 0; });
}

unsigned Board::distributePotNextLevelAmount(const std::vector<unsigned>& contributions) const
{
    for (unsigned c : contributions)
        if (c > 0)
            return c;
    return 0;
}

std::vector<size_t> Board::distributePotEligibleContributors(const std::vector<unsigned>& contributions,
                                                             unsigned levelAmount) const
{
    std::vector<size_t> eligible;
    for (size_t i = 0; i < contributions.size(); ++i)
    {
        if (contributions[i] >= levelAmount && (*std::next(mySeatsList->begin(), i))->getAction() != PlayerActionFold)
            eligible.push_back(i);
    }
    return eligible;
}

std::vector<size_t> Board::distributePotDetermineWinners(const std::vector<size_t>& eligibleIndexes,
                                                         unsigned levelAmount) const
{
    int bestRank = 0;
    for (size_t idx : eligibleIndexes)
    {
        auto& player = *std::next(mySeatsList->begin(), idx);
        bestRank = std::max(bestRank, player->getHandRanking());
    }

    std::vector<size_t> winners;
    for (size_t idx : eligibleIndexes)
    {
        auto& player = *std::next(mySeatsList->begin(), idx);
        if (player->getHandRanking() == bestRank)
            winners.push_back(idx);
    }

    return winners;
}

void Board::distributePotAwardBase(const std::vector<size_t>& winnerIndexes, int baseAmount)
{
    for (size_t idx : winnerIndexes)
    {
        auto& player = *std::next(mySeatsList->begin(), idx);
        player->setCash(player->getCash() + baseAmount);
        player->setLastMoneyWon(player->getLastMoneyWon() + baseAmount);
        myWinners.push_back(player->getId());
    }
}

void Board::distributePotDistributeRemainder(const std::vector<size_t>& winnerIndexes, int remainder)
{
    if (remainder <= 0 || winnerIndexes.empty())
        return;

    std::vector<unsigned> winnerIds;
    for (size_t idx : winnerIndexes)
        winnerIds.push_back((*std::next(mySeatsList->begin(), idx))->getId());

    auto receiver = distributePotResolveRemainderReceiver(winnerIds);
    receiver->setCash(receiver->getCash() + remainder);
    receiver->setLastMoneyWon(receiver->getLastMoneyWon() + remainder);
}

void Board::distributePotReduceContributions(std::vector<unsigned>& contributions, unsigned amount)
{
    for (auto& c : contributions)
        if (c >= amount)
            c -= amount;
}

void Board::distributePotFinalize()
{
    myWinners.sort();
    myWinners.unique();

    if (myPot > 0 && !myWinners.empty())
    {
        int share = myPot / static_cast<int>(myWinners.size());
        for (unsigned id : myWinners)
        {
            auto player = getPlayerById(id);
            player->setCash(player->getCash() + share);
            player->setLastMoneyWon(player->getLastMoneyWon() + share);
        }
        myPot = 0;
    }
}

std::shared_ptr<Player> Board::getPlayerById(unsigned id) const
{
    for (const auto& p : *mySeatsList)
    {
        if (p->getId() == id)
            return p;
    }
    throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
}

std::shared_ptr<Player> Board::distributePotResolveRemainderReceiver(const std::vector<unsigned>& winnerIds) const
{
    auto it = getSeatsIt(myDealerPlayerId);
    do
    {
        ++it;
        if (it == mySeatsList->end())
            it = mySeatsList->begin();
        if (std::find(winnerIds.begin(), winnerIds.end(), (*it)->getId()) != winnerIds.end())
            return *it;
    } while ((*it)->getId() != myDealerPlayerId);

    return getPlayerById(winnerIds.front()); // fallback
}
pkt::core::player::PlayerListIterator Board::getSeatsIt(unsigned playerId) const
{
    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        if ((*it)->getId() == playerId)
            return it;
    }
    throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
}

void Board::determinePlayerNeedToShowCards()
{

    myPlayerNeedToShowCards.clear();

    // in All In Condition everybody have to show the cards
    if (myAllInCondition)
    {

        PlayerListConstIterator itC;

        for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
        {
            if ((*itC)->getAction() != PlayerActionFold)
            {
                myPlayerNeedToShowCards.push_back((*itC)->getId());
            }
        }
    }

    else
    {

        // all winners have to show their cards

        std::list<std::pair<int, int>> level;

        PlayerListConstIterator lastActionPlayerIt;
        PlayerListConstIterator itC;

        // search lastActionPlayer
        for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
        {
            if ((*itC)->getId() == myLastActionPlayerId && (*itC)->getAction() != PlayerActionFold)
            {
                lastActionPlayerIt = itC;
                break;
            }
        }

        if (itC == mySeatsList->end())
        {
            for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
            {
                if ((*itC)->getAction() != PlayerActionFold)
                {
                    lastActionPlayerIt = itC;
                    break;
                }
            }
        }

        // the player who has done the last action has to show his cards first
        myPlayerNeedToShowCards.push_back((*lastActionPlayerIt)->getId());

        std::pair<int, int> levelTmp;
        // get position und cardsValue of the player who show his cards first
        levelTmp.first = (*lastActionPlayerIt)->getHandRanking();
        levelTmp.second = (*lastActionPlayerIt)->getRoundStartCash() - (*lastActionPlayerIt)->getCash();

        level.push_back(levelTmp);

        std::list<std::pair<int, int>>::iterator levelIt;
        std::list<std::pair<int, int>>::iterator nextLevelIt;

        itC = lastActionPlayerIt;
        ++itC;

        for (unsigned i = 0; i < mySeatsList->size(); i++)
        {

            if (itC == mySeatsList->end())
            {
                itC = mySeatsList->begin();
            }

            if ((*itC)->getAction() != PlayerActionFold)
            {

                for (levelIt = level.begin(); levelIt != level.end(); ++levelIt)
                {
                    if ((*itC)->getHandRanking() > (*levelIt).first)
                    {
                        nextLevelIt = levelIt;
                        ++nextLevelIt;
                        if (nextLevelIt == level.end())
                        {
                            myPlayerNeedToShowCards.push_back((*itC)->getId());
                            levelTmp.first = (*itC)->getHandRanking();
                            levelTmp.second = (*itC)->getRoundStartCash() - (*itC)->getCash();
                            level.push_back(levelTmp);
                            break;
                        }
                    }
                    else
                    {
                        if ((*itC)->getHandRanking() == (*levelIt).first)
                        {
                            nextLevelIt = levelIt;
                            ++nextLevelIt;

                            if (nextLevelIt == level.end() ||
                                (*itC)->getRoundStartCash() - (*itC)->getCash() > (*nextLevelIt).second)
                            {
                                myPlayerNeedToShowCards.push_back((*itC)->getId());
                                if ((*itC)->getRoundStartCash() - (*itC)->getCash() > (*levelIt).second)
                                {
                                    (*levelIt).second = (*itC)->getRoundStartCash() - (*itC)->getCash();
                                }
                            }
                            break;
                        }
                        else
                        {
                            if ((*itC)->getRoundStartCash() - (*itC)->getCash() > (*levelIt).second)
                            {
                                myPlayerNeedToShowCards.push_back((*itC)->getId());
                                levelTmp.first = (*itC)->getHandRanking();
                                levelTmp.second = (*itC)->getRoundStartCash() - (*itC)->getCash();

                                level.insert(levelIt, levelTmp);

                                break;
                            }
                        }
                    }
                }
            }

            ++itC;
        }

        level.clear();
    }

    // sort and unique the list
    myPlayerNeedToShowCards.sort();
    myPlayerNeedToShowCards.unique();
}
void Board::setCards(int* theValue)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        myCards[i] = theValue[i];
    }
}
void Board::getCards(int* theValue)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        theValue[i] = myCards[i];
    }
}

void Board::setAllInCondition(bool theValue)
{
    myAllInCondition = theValue;
}
void Board::setLastActionPlayerId(unsigned theValue)
{
    myLastActionPlayerId = theValue;
}

int Board::getPot() const
{
    return myPot;
}
void Board::setPot(int theValue)
{
    myPot = theValue;
}
int Board::getSets() const
{
    return mySets;
}
void Board::setSets(int theValue)
{
    mySets = theValue;
}

std::list<unsigned> Board::getWinners() const
{
    return myWinners;
}
void Board::setWinners(const std::list<unsigned>& w)
{
    myWinners = w;
}

std::list<unsigned> Board::getPlayerNeedToShowCards() const
{
    return myPlayerNeedToShowCards;
}
void Board::setPlayerNeedToShowCards(const std::list<unsigned>& p)
{
    myPlayerNeedToShowCards = p;
}
} // namespace pkt::core
