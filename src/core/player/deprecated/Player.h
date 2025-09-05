// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/HandEvaluator.h>
#include <core/interfaces/IHand.h>
#include <core/player/PlayerStatistics.h>
#include "core/engine/deprecated/ButtonState.h"

#include "CurrentHandActions.h"
#include "range/RangeEstimator.h"
#include "strategy/CurrentHandContext.h"

#include "core/interfaces/IHandEvaluationEngine.h"

#include <array>
#include <assert.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace pkt::core
{
class IHand;
enum ButtonState;

} // namespace pkt::core

namespace pkt::core::player
{

class BotStrategy;

class Player
{
  public:
    Player(const GameEvents&, int id, std::string name, int sC, bool aS, ButtonState b);

    virtual ~Player() = default;

    int getId() const;
    void setId(unsigned newId);

    void setName(const std::string& theValue);
    std::string getName() const;

    void setCash(int theValue);
    int getCash() const;
    void addBetAmount(int theValue);
    void setSetNull();
    int getTotalBetAmount() const;
    int getLastRelativeSet() const;

    void setHand(IHand*);
    IHand* getHand() const { return currentHand; }
    void setAction(PlayerAction theValue, bool blind = 0);
    PlayerAction getAction() const;

    virtual bool isBot() const = 0;

    void setButton(ButtonState theValue);
    ButtonState getButton() const;

    void setCards(int* theValue);
    void getCards(int* theValue) const;

    void setCardsFlip(bool theValue);
    bool getCardsFlip() const;

    void setHandRanking(int theValue);
    int getHandRanking() const;

    void setCashAtHandStart(int theValue);
    int getCashAtHandStart() const;

    void setLastMoneyWon(int theValue);
    int getLastMoneyWon() const;

    std::string getCardsValueString() const;

    const PlayerPosition getPosition() const;
    void setPosition();
    std::string getPositionLabel(PlayerPosition p) const;

    static bool hasPosition(PlayerPosition myPos, PlayerList runningPlayers);

    bool checkIfINeedToShowCards() const;

    float getM() const;

    const PlayerStatistics& getStatistics(const int nbPlayers) const;
    CurrentHandContext& getCurrentHandContext() { return *myCurrentHandContext; };

    virtual float calculatePreflopCallingRange(const CurrentHandContext& ctx) const;

    const PostFlopAnalysisFlags getPostFlopAnalysisFlags() const;
    CurrentHandActions& getCurrentHandActions();

    void updatePreflopStatistics();
    void updateFlopStatistics();
    void updateTurnStatistics();
    void updateRiverStatistics();

    int getPotOdd() const;
    void setPreflopPotOdd(const int potOdd);

    std::string getStringBoard() const;

    bool isAgressor(const GameState gameState) const;

    void updateCurrentHandContext(const GameState gameState);

    bool isInVeryLooseMode(const int nbPlayers) const;

    std::unique_ptr<RangeEstimator>& getRangeEstimator() { return myRangeEstimator; }
    int getPreflopPotOdd() const;

  protected:
    void loadStatistics();
    void resetPlayerStatistics();
    float getMaxOpponentsStrengths() const;
    bool isPreflopBigBet() const;
    float getOpponentWinningHandsPercentage(const int idPlayer, std::string board) const;

    bool canBluff(const GameState) const;

    // attributes

    IHand* currentHand{nullptr};
    const GameEvents& myEvents;

    CurrentHandActions myCurrentHandActions;
    std::unique_ptr<CurrentHandContext> myCurrentHandContext;

    // const
    int myID;
    std::string myName;

    // vars
    PlayerPosition myPosition;
    std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> myStatistics;
    int myHandRanking{0};

    int myCards[2];
    std::string myCard1;
    std::string myCard2;

    int myCash{0};
    int myCashAtHandStart{0};

    /**
     * Purpose: Represents the total amount of chips the player has committed to the pot during the game.
     * Usage:
     * Tracks the cumulative amount the player has bet across all rounds.
     * Used in pot distribution calculations to determine the player's share of the pot.
     * Example:
     * If a player bets 200 in the preflop and 300 on the flop, myTotalBetAmount would be 500.
     */
    int myTotalBetAmount{0};

    /**
     * Purpose: Represents the amount of chips the player has committed to the pot during the most recent betting
     * action. Usage: Tracks the player's contribution during the current betting action (e.g., a call, raise, or
     * all-in). Helps calculate the player's odds and contributions for the current pot level. Example: If a player
     * raises by 300 chips, myLastRelativeSet would be set to 300
     */
    int myLastRelativeSet{0};

    PlayerAction myAction{-1, ActionType::None, 0};
    ButtonState myButton{ButtonState::Unspecified};

    bool myCardsFlip{0}; // 0 = cards are not fliped, 1 = cards are already flipped,
    int lastMoneyWon{0};
    int myPreflopPotOdd;
    std::unique_ptr<RangeEstimator> myRangeEstimator;

  private:
    std::map<int, float> evaluateOpponentsStrengths() const;
    const HandSimulationStats computeHandSimulation() const;
};
} // namespace pkt::core::player
