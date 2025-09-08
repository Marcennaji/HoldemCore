// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/IBettingRound.h"
#include "core/interfaces/IHand.h"
#include "core/player/deprecated/HumanPlayer.h"

namespace pkt::core
{
class GameEvents;

class BettingRound : public IBettingRound
{
  public:
    BettingRound(const GameEvents&, IHand* hand, unsigned dealerPosition, int smallBlind, GameState);
    ~BettingRound();

    GameState getBettingRoundId() const;

    void setMinimumRaise(int);
    int getMinimumRaise() const;
    void setFullBetRule(bool);
    bool getFullBetRule() const;
    void giveActionToNextBotPlayer();
    void run();

  protected:
    IHand* getHand() const;
    void findLastActivePlayerBeforeSmallBlind();
    bool checkAllHighestSet();
    void setCurrentPlayerTurnId(int theValue);
    int getCurrentPlayerTurnId() const;

    void setFirstRoundLastPlayersTurnId(int theValue);
    int getFirstRoundLastPlayersTurnId() const;

    void setHighestSet(int theValue);
    int getRoundHighestSet() const;

    void setFirstRun(bool theValue);
    bool getFirstRun() const;

    void setFirstRound(bool theValue);
    bool getFirstRound() const;

    int getSmallBlindPlayerId() const;

    int getBigBlindPlayerId() const;

    void setSmallBlind(int theValue);
    int getSmallBlind() const;

    const GameEvents& myEvents;

  private:
    // helpers
    unsigned findNextEligiblePlayerFromSmallBlind();
    void handleFirstRun();
    unsigned getPreviousActingPlayerId(unsigned currentPlayerId, pkt::core::player::PlayerList actingPlayersList) const;
    void logBoardCards();
    bool allBetsAreDone() const;
    void proceedToNextBettingRound();

    IHand* myHand;

    const GameState myBettingRoundId;
    int myDealerPlayerId{0};

    int mySmallBlindPlayerId{0};
    int myBigBlindPlayerId{0};

    int mySmallBlind;
    int myHighestSet{0};
    int myMinimumRaise;
    bool myFullBetRule{false};

    bool myFirstRun{true};
    bool myFirstRunGui{true}; // HACK
    bool myFirstRound{true};

    int myCurrentPlayerTurnId{0};
    int myFirstRoundLastPlayersTurnId{0};

    bool myLogBoardCardsDone{false};
};

} // namespace pkt::core
