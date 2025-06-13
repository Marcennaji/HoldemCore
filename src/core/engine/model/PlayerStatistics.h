/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#pragma once

#include "PlayerAction.h"

#include <deque>

namespace pkt::core
{

class PreflopStatistics
{

  public:
    float getVoluntaryPutMoneyInPot() const;
    float getPreflopRaise() const;
    float getPreflop3Bet() const;
    float getPreflop4Bet() const;
    float getPreflopCall3BetsFrequency() const;
    void AddLastAction(PlayerAction);
    int GetLastActionsNumber(PlayerAction p) const;

    void reset();

    long m_hands;
    long m_folds;
    long m_limps;
    long m_checks;
    long m_calls;
    long m_raises;
    long m_3Bets;
    long m_call3Bets;
    long m_call3BetsOpportunities;
    long m_4Bets;

    static const int LAST_ACTIONS_STACK_SIZE;

    // actions for last n hands :
    std::deque<PlayerAction> m_lastActions;
};

class FlopStatistics
{

  public:
    float getAgressionFactor() const;
    float getAgressionFrequency() const;
    float getContinuationBetFrequency() const;

    void reset();

    long m_hands;
    long m_folds;
    long m_checks;
    long m_calls;
    long m_raises;
    long m_3Bets;
    long m_4Bets;
    long m_bets;
    long m_continuationBetsOpportunities;
    long m_continuationBets;
};

class TurnStatistics
{

  public:
    void reset();

    float getAgressionFactor() const;
    float getAgressionFrequency() const;

    long m_hands;
    long m_folds;
    long m_checks;
    long m_calls;
    long m_bets;
    long m_raises;
    long m_3Bets;
    long m_4Bets;
};

class RiverStatistics
{

  public:
    void reset();

    float getAgressionFactor() const;
    float getAgressionFrequency() const;

    long m_hands;
    long m_folds;
    long m_checks;
    long m_calls;
    long m_bets;
    long m_raises;
    long m_3Bets;
    long m_4Bets;
};

class PlayerStatistics
{
  public:
    PlayerStatistics() { reset(); }
    ~PlayerStatistics() {}

    const PreflopStatistics& getPreflopStatistics() const;
    const FlopStatistics& getFlopStatistics() const;
    const TurnStatistics& getTurnStatistics() const;
    const RiverStatistics& getRiverStatistics() const;
    float getWentToShowDown() const;

    void reset(); // init to zero

    long m_toTalHands;
    PreflopStatistics m_preflopStatistics;
    FlopStatistics m_flopStatistics;
    TurnStatistics m_turnStatistics;
    RiverStatistics m_riverStatistics;
};
} // namespace pkt::core
