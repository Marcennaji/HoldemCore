// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Board.h"

#include "core/interfaces/IHand.h"

#include <core/interfaces/ILogger.h>
#include "Exception.h"
#include "Player.h"
#include "model/EngineError.h"

#include <algorithm>

namespace pkt::core
{
using namespace pkt::core::player;

Board::Board(unsigned dp) : IBoard(), pot(0), sets(0), dealerPosition(dp), allInCondition(false), lastActionPlayerID(0)
{
    myCards[0] = myCards[1] = myCards[2] = myCards[3] = myCards[4] = 0;
}

Board::~Board()
{
    if (seatsList)
    {
        seatsList->clear();
    }
    if (activePlayerList)
    {
        activePlayerList->clear();
    }
    if (runningPlayerList)
    {
        runningPlayerList->clear();
    }
}
void Board::setPlayerLists(PlayerList sl, PlayerList apl, PlayerList rpl)
{
    seatsList = sl;
    activePlayerList = apl;
    runningPlayerList = rpl;
}

void Board::collectSets()
{

    sets = 0;

    PlayerListConstIterator it_c;
    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {
        sets += (*it_c)->getSet();
    }
}

void Board::collectPot()
{

    pot += sets;
    sets = 0;

    PlayerListIterator it;
    for (it = seatsList->begin(); it != seatsList->end(); ++it)
    {
        (*it)->setSetNull();
    }
}

void Board::distributePot()
{

    winners.clear();

    size_t i, j, k, l;
    PlayerListIterator it;
    PlayerListConstIterator it_c;

    // filling player sets vector
    std::vector<unsigned> playerSets;
    for (it = seatsList->begin(); it != seatsList->end(); ++it)
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
            for (it_c = seatsList->begin(), j = 0; it_c != seatsList->end(); ++it_c, j++)
            {
                if ((*it_c)->getActiveStatus() && (*it_c)->getCardsValueInt() > highestCardsValue &&
                    (*it_c)->getAction() != PLAYER_ACTION_FOLD && playerSets[j] >= potLevel[0])
                {
                    highestCardsValue = (*it_c)->getCardsValueInt();
                }
            }

            // level winners
            for (it_c = seatsList->begin(), j = 0; it_c != seatsList->end(); ++it_c, j++)
            {
                if ((*it_c)->getActiveStatus() && highestCardsValue == (*it_c)->getCardsValueInt() &&
                    (*it_c)->getAction() != PLAYER_ACTION_FOLD && playerSets[j] >= potLevel[0])
                {
                    potLevel.push_back((*it_c)->getID());
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
                    for (it = seatsList->begin(); it != seatsList->end(); ++it)
                    {
                        if ((*it)->getID() == potLevel[j])
                        {
                            break;
                        }
                    }
                    if (it == seatsList->end())
                    {
                        throw Exception(__FILE__, __LINE__, EngineError::SEAT_NOT_FOUND);
                    }
                    (*it)->setCash((*it)->getCash() + ((potLevel[1]) / winnerCount));

                    // filling winners vector
                    winners.push_back((*it)->getID());
                    (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + (potLevel[1]) / winnerCount);
                }
            }
            // pot level sum not divisible by winnerCount
            // --> distribution after smallBlind
            else
            {

                // find Seat with dealerPosition
                for (it = seatsList->begin(); it != seatsList->end(); ++it)
                {
                    if ((*it)->getID() == dealerPosition)
                    {
                        break;
                    }
                }
                if (it == seatsList->end())
                {
                    throw Exception(__FILE__, __LINE__, EngineError::SEAT_NOT_FOUND);
                }

                for (j = 0; j < winnerCount; j++)
                {

                    winnerHit = false;

                    for (k = 0; k < MAX_NUMBER_OF_PLAYERS && !winnerHit; k++)
                    {

                        ++it;
                        if (it == seatsList->end())
                            it = seatsList->begin();

                        for (l = 2; l < potLevel.size(); l++)
                        {
                            if ((*it)->getID() == potLevel[l])
                                winnerHit = true;
                        }
                    }

                    if (j < mod)
                    {
                        (*it)->setCash((*it)->getCash() + (int) ((potLevel[1]) / winnerCount) + 1);
                        // filling winners vector
                        winners.push_back((*it)->getID());
                        (*it)->setLastMoneyWon((*it)->getLastMoneyWon() + ((potLevel[1]) / winnerCount) + 1);
                    }
                    else
                    {
                        (*it)->setCash((*it)->getCash() + (int) ((potLevel[1]) / winnerCount));
                        // filling winners vector
                        winners.push_back((*it)->getID());
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
            pot -= potLevel[1];

            if (pot == 0)
                break;

            // clear potLevel
            potLevel.clear();
        }
    }

    // winners sort and unique
    winners.sort();
    winners.unique();

    if (pot != 0)
    {

        std::list<unsigned>::iterator it_int;

        for (it_int = winners.begin(); it_int != winners.end(); ++it_int)
        {

            for (it = seatsList->begin(); it != seatsList->end(); ++it)
            {
                if ((*it)->getID() == (*it_int))
                    (*it)->setCash((*it)->getCash() + (pot / winners.size()));
            }
        }
    }
}

void Board::determinePlayerNeedToShowCards()
{

    playerNeedToShowCards.clear();

    // in All In Condition everybody have to show the cards
    if (allInCondition)
    {

        PlayerListConstIterator it_c;

        for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
        {
            if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
            {
                playerNeedToShowCards.push_back((*it_c)->getID());
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
        for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
        {
            if ((*it_c)->getID() == lastActionPlayerID && (*it_c)->getAction() != PLAYER_ACTION_FOLD)
            {
                lastActionPlayerIt = it_c;
                break;
            }
        }

        if (it_c == activePlayerList->end())
        {
            for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
            {
                if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
                {
                    lastActionPlayerIt = it_c;
                    break;
                }
            }
        }

        // the player who has done the last action has to show his cards first
        playerNeedToShowCards.push_back((*lastActionPlayerIt)->getID());

        std::pair<int, int> level_tmp;
        // get position und cardsValue of the player who show his cards first
        level_tmp.first = (*lastActionPlayerIt)->getCardsValueInt();
        level_tmp.second = (*lastActionPlayerIt)->getRoundStartCash() - (*lastActionPlayerIt)->getCash();

        level.push_back(level_tmp);

        std::list<std::pair<int, int>>::iterator level_it;
        std::list<std::pair<int, int>>::iterator next_level_it;

        it_c = lastActionPlayerIt;
        ++it_c;

        for (unsigned i = 0; i < activePlayerList->size(); i++)
        {

            if (it_c == activePlayerList->end())
                it_c = activePlayerList->begin();

            if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
            {

                for (level_it = level.begin(); level_it != level.end(); ++level_it)
                {
                    if ((*it_c)->getCardsValueInt() > (*level_it).first)
                    {
                        next_level_it = level_it;
                        ++next_level_it;
                        if (next_level_it == level.end())
                        {
                            playerNeedToShowCards.push_back((*it_c)->getID());
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
                                playerNeedToShowCards.push_back((*it_c)->getID());
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
                                playerNeedToShowCards.push_back((*it_c)->getID());
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
    playerNeedToShowCards.sort();
    playerNeedToShowCards.unique();
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
    allInCondition = theValue;
}
void Board::setLastActionPlayerID(unsigned theValue)
{
    lastActionPlayerID = theValue;
}

int Board::getPot() const
{
    return pot;
}
void Board::setPot(int theValue)
{
    pot = theValue;
}
int Board::getSets() const
{
    return sets;
}
void Board::setSets(int theValue)
{
    sets = theValue;
}

std::list<unsigned> Board::getWinners() const
{
    return winners;
}
void Board::setWinners(const std::list<unsigned>& w)
{
    winners = w;
}

std::list<unsigned> Board::getPlayerNeedToShowCards() const
{
    return playerNeedToShowCards;
}
void Board::setPlayerNeedToShowCards(const std::list<unsigned>& p)
{
    playerNeedToShowCards = p;
}
} // namespace pkt::core
