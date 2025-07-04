// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/interfaces/IBettingRound.h"
#include "core/interfaces/IHand.h"
#include "core/player/HumanPlayer.h"

namespace pkt::core
{
class GameEvents;

class BettingRound : public IBettingRound
{
  public:
    BettingRound(const GameEvents&, IHand* hi, unsigned dP, int sB, GameState gS);
    ~BettingRound();

    GameState getBettingRoundId() const;
    int getHighestCardsValue() const;
    void setHighestCardsValue(int);
    void setMinimumRaise(int);
    int getMinimumRaise() const;
    void setFullBetRule(bool);
    bool getFullBetRule() const;
    void skipFirstRunGui();
    void nextPlayer();
    void run();
    void postRiverRun();

  protected:
    IHand* getHand() const;

    void setCurrentPlayersTurnId(unsigned theValue);
    unsigned getCurrentPlayersTurnId() const;

    void setFirstRoundLastPlayersTurnId(unsigned theValue);
    unsigned getFirstRoundLastPlayersTurnId() const;

    void setCurrentPlayersTurnIt(pkt::core::player::PlayerListIterator theValue);
    pkt::core::player::PlayerListIterator getCurrentPlayersTurnIt() const;

    void setLastPlayersTurnIt(pkt::core::player::PlayerListIterator theValue);
    pkt::core::player::PlayerListIterator getLastPlayersTurnIt() const;

    void setHighestSet(int theValue);
    int getHighestSet() const;

    void setFirstRun(bool theValue);
    bool getFirstRun() const;

    void setFirstRound(bool theValue);
    bool getFirstRound() const;

    unsigned getSmallBlindPositionId() const;

    void setBigBlindPositionId(unsigned theValue);
    unsigned getBigBlindPositionId() const;

    void setSmallBlind(int theValue);
    int getSmallBlind() const;

    const GameEvents& myEvents;

  private:
    // helpers
    unsigned findNextEligiblePlayerFromSmallBlind();
    void handleFirstRun();
    unsigned getPreviousRunningPlayerId(unsigned currentPlayerId,
                                        pkt::core::player::PlayerList runningPlayersList) const;
    void logBoardCards();
    bool allBetsAreDone() const;
    void proceedToNextBettingRound();

    IHand* myHand;

    const GameState myBettingRoundId;
    int myDealerPosition{0};

    unsigned mySmallBlindPlayerId{0};
    unsigned myBigBlindPositionId{0};

    int mySmallBlind;
    int myHighestSet{0};
    int myMinimumRaise;
    bool myFullBetRule{false};

    bool myFirstRun{true};
    bool myFirstRunGui{true}; // HACK
    bool myFirstRound{true};
    bool myFirstHeadsUpRound{true};

    pkt::core::player::PlayerListIterator myCurrentPlayersTurnIt; // iterator for runningPlayerList
    pkt::core::player::PlayerListIterator myLastPlayersTurnIt;    // iterator for runningPlayerList

    unsigned myCurrentPlayersTurnId{0};
    unsigned myFirstRoundLastPlayersTurnId{0};

    bool myLogBoardCardsDone{false};
};

} // namespace pkt::core
