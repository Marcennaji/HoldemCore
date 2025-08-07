#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <gtest/gtest.h>
#include "StrategyTest.h"
#include "core/interfaces/NullLogger.h"

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

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
    ctx.commonContext.gameState = GameStatePreflop;
    ctx.perPlayerContext.myCard1 = "Ah";
    ctx.perPlayerContext.myCard2 = "Ad";
    EXPECT_GT(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_SuitedBroadway_Raises)
{
    ctx.commonContext.gameState = GameStatePreflop;
    ctx.perPlayerContext.myCard1 = "Ks";
    ctx.perPlayerContext.myCard2 = "Qs";
    EXPECT_GT(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_Trash_DoesNotRaise)
{
    ctx.commonContext.gameState = GameStatePreflop;
    ctx.perPlayerContext.myCard1 = "7d";
    ctx.perPlayerContext.myCard2 = "2c";
    EXPECT_EQ(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_InPosition_CallsWithOdds)
{
    ctx.commonContext.gameState = GameStatePreflop;
    ctx.perPlayerContext.myCard1 = "8h";
    ctx.perPlayerContext.myCard2 = "9h";
    ctx.perPlayerContext.myPosition = PlayerPosition::BUTTON;
    ctx.perPlayerContext.myHandSimulation.winSd = 0.6f;
    ctx.commonContext.preflopRaisesNumber = 1;
    ctx.commonContext.preflopLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);
    ctx.perPlayerContext.myHavePosition = true;
    EXPECT_TRUE(strategy.preflopShouldCall(ctx));
}

// --- Flop ---

TEST_F(TightAggressiveStrategyTest, Flop_HighEquity_Raises)
{
    ctx.commonContext.gameState = GameStateFlop;
    ctx.perPlayerContext.myCard1 = "Jh";
    ctx.perPlayerContext.myCard2 = "Js";
    ctx.commonContext.stringBoard = "2h 3d 7c";
    ctx.perPlayerContext.myPosition = PlayerPosition::BUTTON;
    ctx.commonContext.flopBetsOrRaisesNumber = 1;
    ctx.commonContext.flopLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);
    ctx.perPlayerContext.myHavePosition = true;

    ctx.perPlayerContext.myHandSimulation.win = 0.92f;
    ctx.perPlayerContext.myHandSimulation.winSd = 0.95f;
    ctx.perPlayerContext.myHandSimulation.winRanged = 0.9f;

    EXPECT_GT(strategy.flopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Flop_LowEquity_Folds)
{
    ctx.commonContext.gameState = GameStateFlop;
    ctx.perPlayerContext.myHandSimulation.winSd = 0.2f;
    ctx.commonContext.potOdd = 10;
    EXPECT_FALSE(strategy.flopShouldCall(ctx));
}

// --- Turn ---

TEST_F(TightAggressiveStrategyTest, Turn_HighEquity_Raises)
{
    ctx.commonContext.gameState = GameStateTurn;
    ctx.perPlayerContext.myHandSimulation.win = 0.95f;
    ctx.perPlayerContext.myHandSimulation.winRanged = 0.95f;
    ctx.perPlayerContext.myPosition = PlayerPosition::BUTTON;
    ctx.commonContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.turnLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);

    EXPECT_GT(strategy.turnShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_DoesNotRaise)
{
    ctx.commonContext.gameState = GameStateTurn;
    ctx.perPlayerContext.myHandSimulation.win = 0.4f;
    ctx.perPlayerContext.myHandSimulation.winRanged = 0.4f;
    ctx.perPlayerContext.myPosition = PlayerPosition::BUTTON;
    ctx.commonContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.turnLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);

    EXPECT_EQ(strategy.turnShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_Folds)
{
    ctx.commonContext.gameState = GameStateTurn;
    ctx.perPlayerContext.myHandSimulation.winSd = 0.1f;
    ctx.commonContext.potOdd = 10;
    EXPECT_FALSE(strategy.turnShouldCall(ctx));
}

// --- River ---

TEST_F(TightAggressiveStrategyTest, River_Nuts_Raises)
{
    ctx.commonContext.gameState = GameStateRiver;
    ctx.perPlayerContext.myHandSimulation.winSd = 1.0f;
    ctx.perPlayerContext.myHandSimulation.win = 1.0f;
    ctx.perPlayerContext.myHandSimulation.winRanged = 1.0f;
    ctx.perPlayerContext.myPosition = PlayerPosition::BUTTON;
    ctx.commonContext.riverBetsOrRaisesNumber = 5;

    EXPECT_GT(strategy.riverShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, River_ZeroEquity_Folds)
{
    ctx.commonContext.gameState = GameStateRiver;
    ctx.perPlayerContext.myHandSimulation.winSd = 0.0f;
    EXPECT_FALSE(strategy.riverShouldCall(ctx));
}

// --- Edge / Defensive ---

TEST_F(TightAggressiveStrategyTest, ZeroCash_CannotRaise)
{
    ctx.commonContext.gameState = GameStatePreflop;
    ctx.perPlayerContext.myHandSimulation.winSd = 0.5f;
    ctx.perPlayerContext.myCash = 0;
    ctx.perPlayerContext.myCard1 = "8h";
    ctx.perPlayerContext.myCard2 = "9h";
    EXPECT_FALSE(strategy.preflopShouldCall(ctx));
    EXPECT_EQ(strategy.preflopShouldRaise(ctx), 0);
}

TEST_F(TightAggressiveStrategyTest, NoBluff_DisablesBluffing)
{
    ctx.commonContext.gameState = GameStateFlop;
    ctx.perPlayerContext.myHavePosition = true;
    ctx.perPlayerContext.myCanBluff = false;
    ctx.perPlayerContext.myHandSimulation.winSd = 0.4f;
    ctx.perPlayerContext.myCard1 = "8h";
    ctx.perPlayerContext.myCard2 = "9h";
    EXPECT_EQ(strategy.flopShouldRaise(ctx), 0);
}

} // namespace pkt::test
