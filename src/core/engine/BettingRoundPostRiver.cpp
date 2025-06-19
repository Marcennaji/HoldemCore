// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BettingRoundPostRiver.h"
#include "core/interfaces/IHand.h"
#include "core/interfaces/persistence/IHandAuditStore.h"
#include "core/interfaces/persistence/IPlayersStatisticsStore.h"
#include "core/interfaces/persistence/IRankingStore.h"

#include "Player.h"

#include <iostream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

BettingRoundPostRiver::BettingRoundPostRiver(const GameEvents& events, ILogger* logger, IHand* hi, int dP, int sB)
    : BettingRound(events, logger, hi, dP, sB, GAME_STATE_POST_RIVER), highestCardsValue(0)
{
}

BettingRoundPostRiver::~BettingRoundPostRiver()
{
}

void BettingRoundPostRiver::run()
{
}

void BettingRoundPostRiver::postRiverRun()
{

    PlayerListConstIterator it_c;
    PlayerListIterator it;

    // who is the winner
    for (it_c = getHand()->getActivePlayerList()->begin(); it_c != getHand()->getActivePlayerList()->end(); ++it_c)
    {

        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD && (*it_c)->getCardsValueInt() > highestCardsValue)
        {
            highestCardsValue = (*it_c)->getCardsValueInt();
        }
    }

    int potPlayers = 0;

    for (it_c = getHand()->getActivePlayerList()->begin(); it_c != getHand()->getActivePlayerList()->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
        {
            potPlayers++;
        }
    }

    getHand()->getBoard()->determinePlayerNeedToShowCards();

    getHand()->getBoard()->distributePot();

    getHand()->getBoard()->setPot(0);

    bool pauseHand = false;
    int nonfoldPlayersCounter = 0;
    for (it_c = getHand()->getActivePlayerList()->begin(); it_c != getHand()->getActivePlayerList()->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
        {
            nonfoldPlayersCounter++;
            if ((*it_c)->getID() == 0)
            {
                pauseHand = true;
            }
        }
    }

    if (getHand()->getRankingStore())
    {
        getHand()->getRankingStore()->updateRankingGameLosers(getHand()->getActivePlayerList());
        getHand()->getRankingStore()->updateRankingGameWinner(getHand()->getActivePlayerList());
    }
    if (getHand()->getPlayersStatisticsStore())
    {
        getHand()->getPlayersStatisticsStore()->updatePlayersStatistics(getHand()->getActivePlayerList());
    }
    if (myEvents.onPostRiverRunAnimation)
        myEvents.onPostRiverRunAnimation();

    if (getHand()->getCardsShown())
    {
        // show cards for players who didn't fold preflop
        for (it_c = getHand()->getActivePlayerList()->begin(); it_c != getHand()->getActivePlayerList()->end(); ++it_c)
        {

            if ((*it_c)->getCurrentHandActions().getPreflopActions().size() > 0 &&
                (*it_c)->getCurrentHandActions().getPreflopActions().at(0) != PLAYER_ACTION_FOLD)

                if (myEvents.onShowHoleCards)
                    myEvents.onShowHoleCards((*it_c)->getID());
        }
    }
    // if the human player went at showdown with at least one opponent, enable pausing the hand so he can see the
    // results
    if (pauseHand && nonfoldPlayersCounter > 1 && myEvents.onPauseHand)
        myEvents.onPauseHand();
}
void BettingRoundPostRiver::setHighestCardsValue(int theValue)
{
    highestCardsValue = theValue;
}
int BettingRoundPostRiver::getHighestCardsValue() const
{
    return highestCardsValue;
}
} // namespace pkt::core
