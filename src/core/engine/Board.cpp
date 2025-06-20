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

Board::Board(unsigned dp)
    : IBoard(), myPot(0), mySets(0), myDealerPosition(dp), myAllInCondition(false), myLastActionPlayerId(0)
{
    myCards[0] = myCards[1] = myCards[2] = myCards[3] = myCards[4] = 0;
}

Board::~Board()
{
    if (mySeatsList)
    {
        mySeatsList->clear();
    }
    if (myActivePlayerList)
    {
        myActivePlayerList->clear();
    }
    if (myRunningPlayerList)
    {
        myRunningPlayerList->clear();
    }
}
void Board::setPlayerLists(PlayerList sl, PlayerList apl, PlayerList rpl)
{
    mySeatsList = sl;
    myActivePlayerList = apl;
    myRunningPlayerList = rpl;
}

void Board::collectSets()
{

    mySets = 0;

    PlayerListConstIterator it_c;
    for (it_c = mySeatsList->begin(); it_c != mySeatsList->end(); ++it_c)
    {
        mySets += (*it_c)->getSet();
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

    size_t i, j, k, l;
    PlayerListIterator it;
    PlayerListConstIterator it_c;

    // filling player sets vector
    std::vector<unsigned> playerSets;
    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        if ((*it)->getActiveStatus())
        {
            playerSets.push_back((((*it)->getRoundStartCash()) - ((*it)->getCash())));
        }
        else
        {
            playerSets.push_back(0);
        }
        (*it)->setLastMoneyWon(0);
    }

    // sort player sets asc
    std::vector<unsigned> playerSetsSort = playerSets;
    std::sort(playerSetsSort.begin(), playerSetsSort.end());

    // potLevel[0] = amount, potLevel[1] = sum, potLevel[2..n] = winner
    std::vector<unsigned> potLevel;

    // temp var
    int highestCardsValue;
    int winnerCount;
    int mod;
    bool winnerHit;

    // level loop
    for (i = 0; i < playerSetsSort.size(); i++)
    {

        // restart levelHighestCardsValue
        highestCardsValue = 0;

        // level detection
        if (playerSetsSort[i] > 0)
        {

            // level amount
            potLevel.push_back(playerSetsSort[i]);

            // level sum
            potLevel.push_back((playerSetsSort.size() - i) * potLevel[0]);

            // determine level highestCardsValue
            for (it_c = mySeatsList->begin(), j = 0; it_c != mySeatsList->end(); ++it_c, j++)
            {
                if ((*it_c)->getActiveStatus() && (*it_c)->getCardsValueInt() > highestCardsValue &&
                    (*it_c)->getAction() != PlayerActionFold && playerSets[j] >= potLevel[0])
                {
                    highestCardsValue = (*it_c)->getCardsValueInt();
                }
            }

            // level winners
            for (it_c = mySeatsList->begin(), j = 0; it_c != mySeatsList->end(); ++it_c, j++)
            {
                if ((*it_c)->getActiveStatus() && highestCardsValue == (*it_c)->getCardsValueInt() &&
                    (*it_c)->getAction() != PlayerActionFold && playerSets[j] >= potLevel[0])
                {
                    potLevel.push_back((*it_c)->getId());
                }
            }

            // determine the number of level winners
            winnerCount = potLevel.size() - 2;

            if (winnerCount == 0 || potLevel.size() == 0)
                break;

            // distribute the pot level sum to level winners
            mod = (potLevel[1]) % winnerCount;
            // pot level sum divisible by winnerCount
            if (mod == 0)
            {

                for (j = 2; j < potLevel.size(); j++)
                {
                    // find seat with potLevel[j]-ID
                    for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
                    {
                        if ((*it)->getId() == potLevel[j])
                        {
                            break;
                        }
                    }
                    if (it == mySeatsList->end())
                    {
                        throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
                    }
                    (*it)->setCash((*it)->getCash() + ((potLevel[1]) / winnerCount));

                    // filling winners vector
                    myWinners.push_back((*it)->getId());
                    (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + (potLevel[1]) / winnerCount);
                }
            }
            // pot level sum not divisible by winnerCount
            // --> distribution after smallBlind
            else
            {

                // find Seat with dealerPosition
                for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
                {
                    if ((*it)->getId() == myDealerPosition)
                    {
                        break;
                    }
                }
                if (it == mySeatsList->end())
                {
                    throw Exception(__FILE__, __LINE__, EngineError::SeatNotFound);
                }

                for (j = 0; j < winnerCount; j++)
                {

                    winnerHit = false;

                    for (k = 0; k < MAX_NUMBER_OF_PLAYERS && !winnerHit; k++)
                    {

                        ++it;
                        if (it == mySeatsList->end())
                            it = mySeatsList->begin();

                        for (l = 2; l < potLevel.size(); l++)
                        {
                            if ((*it)->getId() == potLevel[l])
                                winnerHit = true;
                        }
                    }

                    if (j < mod)
                    {
                        (*it)->setCash((*it)->getCash() + (int) ((potLevel[1]) / winnerCount) + 1);
                        // filling winners vector
                        myWinners.push_back((*it)->getId());
                        (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + ((potLevel[1]) / winnerCount) + 1);
                    }
                    else
                    {
                        (*it)->setCash((*it)->getCash() + (int) ((potLevel[1]) / winnerCount));
                        // filling winners vector
                        myWinners.push_back((*it)->getId());
                        (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + (potLevel[1]) / winnerCount);
                    }
                }
            }

            // reevaluate the player sets
            for (j = 0; j < playerSets.size(); j++)
            {
                if (playerSets[j] > 0)
                {
                    playerSets[j] -= potLevel[0];
                }
            }

            // sort player sets asc
            playerSetsSort = playerSets;
            sort(playerSetsSort.begin(), playerSetsSort.end());

            // pot refresh
            myPot -= potLevel[1];

            if (myPot == 0)
                break;

            // clear potLevel
            potLevel.clear();
        }
    }

    // winners sort and unique
    myWinners.sort();
    myWinners.unique();

    if (myPot != 0)
    {

        std::list<unsigned>::iterator it_int;

        for (it_int = myWinners.begin(); it_int != myWinners.end(); ++it_int)
        {

            for (it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
            {
                if ((*it)->getId() == (*it_int))
                    (*it)->setCash((*it)->getCash() + (myPot / myWinners.size()));
            }
        }
    }
}

void Board::determinePlayerNeedToShowCards()
{

    myPlayerNeedToShowCards.clear();

    // in All In Condition everybody have to show the cards
    if (myAllInCondition)
    {

        PlayerListConstIterator it_c;

        for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
        {
            if ((*it_c)->getAction() != PlayerActionFold)
            {
                myPlayerNeedToShowCards.push_back((*it_c)->getId());
            }
        }
    }

    else
    {

        // all winners have to show their cards

        std::list<std::pair<int, int>> level;

        PlayerListConstIterator lastActionPlayerIt;
        PlayerListConstIterator it_c;

        // search lastActionPlayer
        for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
        {
            if ((*it_c)->getId() == myLastActionPlayerId && (*it_c)->getAction() != PlayerActionFold)
            {
                lastActionPlayerIt = it_c;
                break;
            }
        }

        if (it_c == myActivePlayerList->end())
        {
            for (it_c = myActivePlayerList->begin(); it_c != myActivePlayerList->end(); ++it_c)
            {
                if ((*it_c)->getAction() != PlayerActionFold)
                {
                    lastActionPlayerIt = it_c;
                    break;
                }
            }
        }

        // the player who has done the last action has to show his cards first
        myPlayerNeedToShowCards.push_back((*lastActionPlayerIt)->getId());

        std::pair<int, int> level_tmp;
        // get position und cardsValue of the player who show his cards first
        level_tmp.first = (*lastActionPlayerIt)->getCardsValueInt();
        level_tmp.second = (*lastActionPlayerIt)->getRoundStartCash() - (*lastActionPlayerIt)->getCash();

        level.push_back(level_tmp);

        std::list<std::pair<int, int>>::iterator level_it;
        std::list<std::pair<int, int>>::iterator next_level_it;

        it_c = lastActionPlayerIt;
        ++it_c;

        for (unsigned i = 0; i < myActivePlayerList->size(); i++)
        {

            if (it_c == myActivePlayerList->end())
                it_c = myActivePlayerList->begin();

            if ((*it_c)->getAction() != PlayerActionFold)
            {

                for (level_it = level.begin(); level_it != level.end(); ++level_it)
                {
                    if ((*it_c)->getCardsValueInt() > (*level_it).first)
                    {
                        next_level_it = level_it;
                        ++next_level_it;
                        if (next_level_it == level.end())
                        {
                            myPlayerNeedToShowCards.push_back((*it_c)->getId());
                            level_tmp.first = (*it_c)->getCardsValueInt();
                            level_tmp.second = (*it_c)->getRoundStartCash() - (*it_c)->getCash();
                            level.push_back(level_tmp);
                            break;
                        }
                    }
                    else
                    {
                        if ((*it_c)->getCardsValueInt() == (*level_it).first)
                        {
                            next_level_it = level_it;
                            ++next_level_it;

                            if (next_level_it == level.end() ||
                                (*it_c)->getRoundStartCash() - (*it_c)->getCash() > (*next_level_it).second)
                            {
                                myPlayerNeedToShowCards.push_back((*it_c)->getId());
                                if ((*it_c)->getRoundStartCash() - (*it_c)->getCash() > (*level_it).second)
                                {
                                    (*level_it).second = (*it_c)->getRoundStartCash() - (*it_c)->getCash();
                                }
                            }
                            break;
                        }
                        else
                        {
                            if ((*it_c)->getRoundStartCash() - (*it_c)->getCash() > (*level_it).second)
                            {
                                myPlayerNeedToShowCards.push_back((*it_c)->getId());
                                level_tmp.first = (*it_c)->getCardsValueInt();
                                level_tmp.second = (*it_c)->getRoundStartCash() - (*it_c)->getCash();

                                level.insert(level_it, level_tmp);

                                break;
                            }
                        }
                    }
                }
            }

            ++it_c;
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
        myCards[i] = theValue[i];
}
void Board::getCards(int* theValue)
{
    int i;
    for (i = 0; i < 5; i++)
        theValue[i] = myCards[i];
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
