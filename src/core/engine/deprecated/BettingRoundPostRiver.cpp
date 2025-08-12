// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "core/engine/deprecated/BettingRoundPostRiver.h"
#include "core/interfaces/IHand.h"
#include "core/services/GlobalServices.h"

#include "core/player/deprecated/Player.h"

#include <iostream>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

BettingRoundPostRiver::BettingRoundPostRiver(const GameEvents& events, IHand* hi, int dP, int sB)
    : BettingRound(events, hi, dP, sB, PostRiver)
{
}

BettingRoundPostRiver::~BettingRoundPostRiver() = default;

void BettingRoundPostRiver::run()
{
    PlayerListConstIterator itC;
    PlayerListIterator it;

    // who is the winner
    for (itC = getHand()->getSeatsList()->begin(); itC != getHand()->getSeatsList()->end(); ++itC)
    {

        if ((*itC)->getAction() != ActionType::Fold && (*itC)->getHandRanking() > myHighestCardsValue)
        {
            myHighestCardsValue = (*itC)->getHandRanking();
        }
    }

    int potPlayers = 0;

    for (itC = getHand()->getSeatsList()->begin(); itC != getHand()->getSeatsList()->end(); ++itC)
    {
        if ((*itC)->getAction() != ActionType::Fold)
        {
            potPlayers++;
        }
    }

    getHand()->getBoard()->determinePlayerNeedToShowCards();

    getHand()->getBoard()->distributePot();

    getHand()->getBoard()->setPot(0);

    bool pauseHand = false;
    int nonfoldPlayersCounter = 0;
    for (itC = getHand()->getSeatsList()->begin(); itC != getHand()->getSeatsList()->end(); ++itC)
    {
        if ((*itC)->getAction() != ActionType::Fold)
        {
            nonfoldPlayersCounter++;
            if (!(*itC)->isBot())
            {
                pauseHand = true;
            }
        }
    }

    GlobalServices::instance().rankingStore()->updateRankingGameLosers(getHand()->getSeatsList());
    GlobalServices::instance().rankingStore()->updateRankingGameWinner(getHand()->getSeatsList());
    GlobalServices::instance().playersStatisticsStore()->updatePlayersStatistics(getHand()->getSeatsList());

    if (myEvents.onShowdownStarted)
    {
        myEvents.onShowdownStarted();
    }

    if (getHand()->getCardsShown())
    {
        // show cards for players who didn't fold preflop
        for (itC = getHand()->getSeatsList()->begin(); itC != getHand()->getSeatsList()->end(); ++itC)
        {

            if ((*itC)->getCurrentHandActions().getActions(GameState::Preflop).size() > 0 &&
                (*itC)->getCurrentHandActions().getActions(GameState::Preflop).at(0) != ActionType::Fold)
            {

                if (myEvents.onShowHoleCards)
                {
                    myEvents.onShowHoleCards((*itC)->getId());
                }
            }
        }
    }
    // if the human player went at showdown with at least one opponent, enable pausing the hand so he can see the
    // results
    if (pauseHand && nonfoldPlayersCounter > 1 && myEvents.onPauseHand)
    {
        myEvents.onPauseHand();
    }
}

} // namespace pkt::core
