#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <gtest/gtest.h>
#include "StrategyTest.h"
#include "core/interfaces/NullLogger.h"

using namespace pkt::core;
using namespace pkt::core::player;

class TightAggressiveStrategyTest : public StrategyTest
{
  public:
    TightAggressiveStrategyTest() : StrategyTest(), strategy() {}

  protected:
    TightAggressiveBotStrategy strategy;
    const pkt::core::GameEvents events;
};

// --- Preflop ---

TEST_F(TightAggressiveStrategyTest, Preflop_StrongPair_Raises)
{
    ctx.gameState = GameStatePreflop;
    ctx.myCard1 = "Ah";
    ctx.myCard2 = "Ad";
    EXPECT_GT(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_SuitedBroadway_Raises)
{
    ctx.gameState = GameStatePreflop;
    ctx.myCard1 = "Ks";
    ctx.myCard2 = "Qs";
    EXPECT_GT(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_Trash_DoesNotRaise)
{
    ctx.gameState = GameStatePreflop;
    ctx.myCard1 = "7d";
    ctx.myCard2 = "2c";
    EXPECT_EQ(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_InPosition_CallsWithOdds)
{
    ctx.gameState = GameStatePreflop;
    ctx.myCard1 = "8h";
    ctx.myCard2 = "9h";
    ctx.myPosition = PlayerPosition::BUTTON;
    ctx.myHandSimulation.winSd = 0.6f;
    ctx.preflopRaisesNumber = 1;
    ctx.preflopLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);
    ctx.myHavePosition = true;
    EXPECT_TRUE(strategy.preflopShouldCall(ctx));
}

// --- Flop ---

TEST_F(TightAggressiveStrategyTest, Flop_HighEquity_Raises)
{
    ctx.gameState = GameStateFlop;
    ctx.myCard1 = "Jh";
    ctx.myCard2 = "Js";
    ctx.stringBoard = "2h 3d 7c";
    ctx.myPosition = PlayerPosition::BUTTON;
    ctx.flopBetsOrRaisesNumber = 1;
    ctx.flopLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);
    ctx.myHavePosition = true;

    ctx.myHandSimulation.win = 0.92f;
    ctx.myHandSimulation.winSd = 0.95f;
    ctx.myHandSimulation.winRanged = 0.9f;

    EXPECT_GT(strategy.flopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Flop_LowEquity_Folds)
{
    ctx.gameState = GameStateFlop;
    ctx.myHandSimulation.winSd = 0.2f;
    ctx.potOdd = 10;
    EXPECT_FALSE(strategy.flopShouldCall(ctx));
}

// --- Turn ---

TEST_F(TightAggressiveStrategyTest, Turn_HighEquity_Raises)
{
    ctx.gameState = GameStateTurn;
    ctx.myHandSimulation.win = 0.95f;
    ctx.myHandSimulation.winRanged = 0.95f;
    ctx.myPosition = PlayerPosition::BUTTON;
    ctx.turnBetsOrRaisesNumber = 1;
    ctx.turnLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);

    EXPECT_GT(strategy.turnShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_DoesNotRaise)
{
    ctx.gameState = GameStateTurn;
    ctx.myHandSimulation.win = 0.4f;
    ctx.myHandSimulation.winRanged = 0.4f;
    ctx.myPosition = PlayerPosition::BUTTON;
    ctx.turnBetsOrRaisesNumber = 1;
    ctx.turnLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);

    EXPECT_EQ(strategy.turnShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_Folds)
{
    ctx.gameState = GameStateTurn;
    ctx.myHandSimulation.winSd = 0.1f;
    ctx.potOdd = 10;
    EXPECT_FALSE(strategy.turnShouldCall(ctx));
}

// --- River ---

TEST_F(TightAggressiveStrategyTest, River_Nuts_Raises)
{
    ctx.gameState = GameStateRiver;
    ctx.myHandSimulation.winSd = 1.0f;
    ctx.myHandSimulation.win = 1.0f;
    ctx.myHandSimulation.winRanged = 1.0f;
    ctx.myPosition = PlayerPosition::BUTTON;
    ctx.riverBetsOrRaisesNumber = 5;

    EXPECT_GT(strategy.riverShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, River_ZeroEquity_Folds)
{
    ctx.gameState = GameStateRiver;
    ctx.myHandSimulation.winSd = 0.0f;
    EXPECT_FALSE(strategy.riverShouldCall(ctx));
}

// --- Edge / Defensive ---

TEST_F(TightAggressiveStrategyTest, ZeroCash_CannotRaise)
{
    ctx.gameState = GameStatePreflop;
    ctx.myHandSimulation.winSd = 0.5f;
    ctx.myCash = 0;
    ctx.myCard1 = "8h";
    ctx.myCard2 = "9h";
    EXPECT_FALSE(strategy.preflopShouldCall(ctx));
    EXPECT_EQ(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, NoBluff_DisablesBluffing)
{
    ctx.gameState = GameStateFlop;
    ctx.myHavePosition = true;
    ctx.myCanBluff = false;
    ctx.myHandSimulation.winSd = 0.4f;
    ctx.myCard1 = "8h";
    ctx.myCard2 = "9h";
    EXPECT_EQ(strategy.flopShouldRaise(ctx), 0);
}
