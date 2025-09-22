// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/model/PlayerAction.h"

#include <deque>

namespace pkt::core::player
{

class PreflopStatistics
{

  public:
    float getVoluntaryPutMoneyInPot() const;
    float getPreflopRaise() const;
    float getPreflop3Bet() const;
    float getPreflop4Bet() const;
    float getPreflopCallthreeBetsFrequency() const;
    void addLastAction(PlayerAction);
    int getLastActionsNumber(ActionType p) const;

    void reset();

    long hands;
    long folds;
    long limps;
    long checks;
    long calls;
    long raises;
    long threeBets;
    long callthreeBets;
    long callthreeBetsOpportunities;
    long fourBets;

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

    long hands;
    long folds;
    long checks;
    long calls;
    long raises;
    long threeBets;
    long fourBets;
    long bets;
    long continuationBetsOpportunities;
    long continuationBets;
};

class TurnStatistics
{

  public:
    void reset();

    float getAgressionFactor() const;
    float getAgressionFrequency() const;

    long hands;
    long folds;
    long checks;
    long calls;
    long bets;
    long raises;
    long threeBets;
    long fourBets;
};

class RiverStatistics
{

  public:
    void reset();

    float getAgressionFactor() const;
    float getAgressionFrequency() const;

    long hands;
    long folds;
    long checks;
    long calls;
    long bets;
    long raises;
    long threeBets;
    long fourBets;
};

class PlayerStatistics
{
  public:
    PlayerStatistics() { reset(); }
    ~PlayerStatistics() = default;

    float getWentToShowDown() const;
    void reset(); // init to zero

    long totalHands;
    PreflopStatistics preflopStatistics;
    FlopStatistics flopStatistics;
    TurnStatistics turnStatistics;
    RiverStatistics riverStatistics;
};
} // namespace pkt::core::player
