// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Board.h"

#include "core/interfaces/IHand.h"

#include <core/services/GlobalServices.h>
#include "Exception.h"
#include "Player.h"
#include "Pot.h"
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

    myTotalBetAmounts = 0;

    PlayerListConstIterator itC;
    for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
    {
        myTotalBetAmounts += (*itC)->getTotalBetAmount();
    }
}

void Board::collectPot()
{

    myPot += myTotalBetAmounts;
    myTotalBetAmounts = 0;

    PlayerListIterator it;
    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->setSetNull();
    }
}

void Board::distributePot()
{
    Pot pot(myPot, mySeatsList, myDealerPlayerId);
    pot.distribute();
    myWinners = pot.getWinners();
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
            if ((*itC)->getAction() != ActionType::Fold)
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
            if ((*itC)->getId() == myLastActionPlayerId && (*itC)->getAction() != ActionType::Fold)
            {
                lastActionPlayerIt = itC;
                break;
            }
        }

        if (itC == mySeatsList->end())
        {
            for (itC = mySeatsList->begin(); itC != mySeatsList->end(); ++itC)
            {
                if ((*itC)->getAction() != ActionType::Fold)
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

            if ((*itC)->getAction() != ActionType::Fold)
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
    return myTotalBetAmounts;
}
void Board::setSets(int theValue)
{
    myTotalBetAmounts = theValue;
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
