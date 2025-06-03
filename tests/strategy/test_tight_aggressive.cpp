#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include <gtest/gtest.h>
#include "StrategyTest.h"

using namespace pkt::core;

class TightAggressiveStrategyTest : public StrategyTest
{
  protected:
    TightAggressiveBotStrategy strategy;
};

// --- Preflop ---

TEST_F(TightAggressiveStrategyTest, Preflop_StrongPair_Raises)
{
    ctx.myCard1 = "Ah";
    ctx.myCard2 = "Ad";
    EXPECT_GT(strategy.preflopShouldRaise(ctx, true), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_SuitedBroadway_Raises)
{
    ctx.myCard1 = "Ks";
    ctx.myCard2 = "Qs";
    EXPECT_GT(strategy.preflopShouldRaise(ctx, true), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_Trash_DoesNotRaise)
{
    ctx.myCard1 = "7d";
    ctx.myCard2 = "2c";
    EXPECT_EQ(strategy.preflopShouldRaise(ctx, true), 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_InPosition_CallsWithOdds)
{
    ctx.pot = 100;
    ctx.mySet = 20;
    ctx.highestSet = 20;
    ctx.potOdd = 25;
    ctx.myHavePosition = true;
    EXPECT_TRUE(strategy.preflopShouldCall(ctx));
}

// --- Flop ---

TEST_F(TightAggressiveStrategyTest, Flop_HighEquity_Raises)
{
    ctx.myHandSimulation.winSd = 0.85f;
    EXPECT_TRUE(strategy.flopShouldRaise(ctx));
}

TEST_F(TightAggressiveStrategyTest, Flop_MidEquityInPosition_Bluffs)
{
    ctx.myHandSimulation.winSd = 0.5f;
    ctx.myCanBluff = true;
    ctx.myHavePosition = true;
    EXPECT_TRUE(strategy.flopShouldRaise(ctx));
}

TEST_F(TightAggressiveStrategyTest, Flop_LowEquity_Folds)
{
    ctx.myHandSimulation.winSd = 0.2f;
    ctx.potOdd = 10;
    EXPECT_FALSE(strategy.flopShouldCall(ctx));
}

// --- Turn ---

TEST_F(TightAggressiveStrategyTest, Turn_HighEquity_Raises)
{
    ctx.myHandSimulation.winSd = 0.95f;
    EXPECT_TRUE(strategy.turnShouldRaise(ctx));
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_DoesNotRaise)
{
    ctx.myHandSimulation.winSd = 0.2f;
    EXPECT_FALSE(strategy.turnShouldRaise(ctx));
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_Folds)
{
    ctx.myHandSimulation.winSd = 0.1f;
    ctx.potOdd = 10;
    EXPECT_FALSE(strategy.turnShouldCall(ctx));
}

// --- River ---

TEST_F(TightAggressiveStrategyTest, River_Nuts_Raises)
{
    ctx.myHandSimulation.winSd = 1.0f;
    EXPECT_TRUE(strategy.riverShouldRaise(ctx));
}

TEST_F(TightAggressiveStrategyTest, River_ZeroEquity_Folds)
{
    ctx.myHandSimulation.winSd = 0.0f;
    EXPECT_FALSE(strategy.riverShouldCall(ctx));
}

// --- Edge / Defensive ---

TEST_F(TightAggressiveStrategyTest, ZeroCash_CannotRaise)
{
    ctx.myCash = 0;
    EXPECT_EQ(strategy.preflopShouldRaise(ctx, true), 0);
    EXPECT_FALSE(strategy.turnShouldRaise(ctx));
}

TEST_F(TightAggressiveStrategyTest, NoBluff_DisablesBluffing)
{
    ctx.myCanBluff = false;
    ctx.myHandSimulation.winSd = 0.4f;
    EXPECT_FALSE(strategy.flopShouldRaise(ctx));
}
