// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Board.h"

#include "Exception.h"
#include "Pot.h"
#include "core/player/Player.h"
#include "hand/Hand.h"
#include "model/EngineError.h"

#include <algorithm>

namespace pkt::core
{
using namespace pkt::core::player;

Board::Board(unsigned dp, const GameEvents& events) : myDealerPlayerId(dp), myEvents(events)
{
    myBoardCards.reset(); // Initialize with invalid cards
}

Board::~Board()
{
}

void Board::setSeatsList(PlayerList seats)
{
    mySeatsList = seats;
}
void Board::setActingPlayersList(PlayerList actingPlayers)
{
    myActingPlayersList = actingPlayers;
}

void Board::distributePot(Hand& hand)
{
    int totalPot = 0;

    for (auto& player : *hand.getSeatsList())
    {
        totalPot += player->getCurrentHandActions().getHandTotalBetAmount();
    }
    Pot pot(totalPot, mySeatsList, myDealerPlayerId);
    pot.distribute();
    myWinners = pot.getWinners();

    if (myEvents.onHandCompleted)
        myEvents.onHandCompleted(myWinners, totalPot);
}

void Board::determinePlayerNeedToShowCards()
{

    myPlayerNeedToShowCards.clear();

    // in All In Condition everybody have to show the cards
    if (myAllInCondition)
    {
        for (auto itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
        {
            if ((*itC)->getLastAction().type != ActionType::Fold)
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
            if ((*itC)->getId() == myLastActionPlayerId && (*itC)->getLastAction().type != ActionType::Fold)
            {
                lastActionPlayerIt = itC;
                break;
            }
        }

        if (itC == mySeatsList->end())
        {
            for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
            {
                if ((*itC)->getLastAction().type != ActionType::Fold)
                {
                    lastActionPlayerIt = itC;
                    break;
                }
            }
        }

        // the player who has done the last action has to show his cards first
        myPlayerNeedToShowCards.push_back((*lastActionPlayerIt)->getId());

        std::pair<int, int> levelTmp;
        // get position and cardsValue of the player who show his cards first
        levelTmp.first = (*lastActionPlayerIt)->getHandRanking();
        levelTmp.second = (*lastActionPlayerIt)->getCashAtHandStart() - (*lastActionPlayerIt)->getCash();

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

            if ((*itC)->getLastAction().type != ActionType::Fold)
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
                            levelTmp.second = (*itC)->getCashAtHandStart() - (*itC)->getCash();
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
                                (*itC)->getCashAtHandStart() - (*itC)->getCash() > (*nextLevelIt).second)
                            {
                                myPlayerNeedToShowCards.push_back((*itC)->getId());
                                if ((*itC)->getCashAtHandStart() - (*itC)->getCash() > (*levelIt).second)
                                {
                                    (*levelIt).second = (*itC)->getCashAtHandStart() - (*itC)->getCash();
                                }
                            }
                            break;
                        }
                        else
                        {
                            if ((*itC)->getCashAtHandStart() - (*itC)->getCash() > (*levelIt).second)
                            {
                                myPlayerNeedToShowCards.push_back((*itC)->getId());
                                levelTmp.first = (*itC)->getHandRanking();
                                levelTmp.second = (*itC)->getCashAtHandStart() - (*itC)->getCash();

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

void Board::setBoardCards(const BoardCards& boardCards)
{
    myBoardCards = boardCards;
}

const BoardCards& Board::getBoardCards() const
{
    return myBoardCards;
}

void Board::setAllInCondition(bool theValue)
{
    myAllInCondition = theValue;
}
void Board::setLastActionPlayerId(unsigned theValue)
{
    myLastActionPlayerId = theValue;
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
int Board::getPot(const Hand& hand) const
{
    int totalPot = 0;

    for (auto& player : *hand.getSeatsList())
    {
        totalPot += player->getCurrentHandActions().getHandTotalBetAmount();
    }
    return totalPot;
}
int Board::getSets(const Hand& hand) const
{
    int total = 0;
    GameState currentRound = hand.getGameState();

    for (auto& player : *hand.getSeatsList())
    {
        total += player->getCurrentHandActions().getRoundTotalBetAmount(currentRound);
    }
    return total;
}
} // namespace pkt::core
