// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "core/engine/GameEvents.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/interfaces/IBettingRound.h"
#include "core/interfaces/IBoard.h"

#include <memory>
#include <vector>

namespace pkt::core
{
struct BettingRoundHistory
{
    GameState round;
    std::vector<std::pair<unsigned, pkt::core::PlayerAction>> actions; // playerId, action
};

class IHand
{
  public:
    virtual ~IHand() = default;

    virtual void start() = 0;
    virtual size_t dealBoardCards() = 0;
    virtual void dealHoleCards(size_t lastArrayIndex) = 0;
    virtual void initAndShuffleDeck() = 0;

    virtual pkt::core::player::PlayerList getSeatsList() const = 0;
    virtual pkt::core::player::PlayerList getRunningPlayersList() const = 0;

    virtual std::shared_ptr<IBoard> getBoard() const = 0;
    virtual std::shared_ptr<IBettingRound> getPreflop() const = 0;
    virtual std::shared_ptr<IBettingRound> getFlop() const = 0;
    virtual std::shared_ptr<IBettingRound> getTurn() const = 0;
    virtual std::shared_ptr<IBettingRound> getRiver() const = 0;
    virtual std::shared_ptr<IBettingRound> getCurrentBettingRound() const = 0;

    virtual void setStartQuantityPlayers(int theValue) = 0;
    virtual int getStartQuantityPlayers() const = 0;

    virtual GameState getCurrentRoundStateFsm() const = 0;

    virtual void setCurrentRoundState(GameState theValue) = 0;
    virtual GameState getCurrentRoundState() const = 0;

    virtual GameState getRoundBeforePostRiver() const = 0;

    virtual int getDealerPosition() const = 0;

    virtual void setSmallBlind(int theValue) = 0;
    virtual int getSmallBlind() const = 0;

    virtual void setAllInCondition(bool theValue) = 0;
    virtual bool getAllInCondition() const = 0;

    virtual void setStartCash(int theValue) = 0;
    virtual int getStartCash() const = 0;

    virtual void setPreviousPlayerId(int theValue) = 0;
    virtual int getPreviousPlayerId() const = 0;

    virtual void setLastActionPlayerId(unsigned theValue) = 0;
    virtual unsigned getLastActionPlayerId() const = 0;

    virtual void setCardsShown(bool theValue) = 0;
    virtual bool getCardsShown() const = 0;

    virtual void resolveHandConditions() = 0;

    virtual int getPreflopCallsNumber() = 0;
    virtual int getPreflopRaisesNumber() = 0;
    virtual int getFlopBetsOrRaisesNumber() = 0;
    virtual int getTurnBetsOrRaisesNumber() = 0;
    virtual int getRiverBetsOrRaisesNumber() = 0;

    virtual std::vector<PlayerPosition> getRaisersPositions() = 0;
    virtual std::vector<PlayerPosition> getCallersPositions() = 0;
    virtual int getLastRaiserId() = 0;
    virtual int getPreflopLastRaiserId() = 0;
    virtual void setPreflopLastRaiserId(int id) = 0;
    virtual int getFlopLastRaiserId() = 0;
    virtual void setFlopLastRaiserId(int id) = 0;
    virtual int getTurnLastRaiserId() = 0;
    virtual void setTurnLastRaiserId(int id) = 0;

    virtual pkt::core::player::PlayerListIterator getSeatsIt(unsigned) const = 0;
    virtual pkt::core::player::PlayerListIterator getRunningPlayerIt(unsigned) const = 0;

    void recordPlayerAction(GameState round, unsigned playerId, const pkt::core::PlayerAction& action)
    {
        // Find or create entry for this round
        auto it = std::find_if(myHandActionHistory.begin(), myHandActionHistory.end(),
                               [round](const BettingRoundHistory& h) { return h.round == round; });

        if (it == myHandActionHistory.end())
        {
            myHandActionHistory.push_back({round, {{playerId, action}}});
        }
        else
        {
            it->actions.emplace_back(playerId, action);
        }
    }

    const std::vector<BettingRoundHistory>& getHandActionHistory() const { return myHandActionHistory; }

    std::vector<BettingRoundHistory> myHandActionHistory;

    friend class Game;
    friend class BettingRound;
    friend class BettingRoundPreflop;
    friend class BettingRoundFlop;
    friend class BettingRoundTurn;
    friend class BettingRoundRiver;
    friend class BettingRoundPostRiver;
};
} // namespace pkt::core
