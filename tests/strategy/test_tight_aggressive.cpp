#include <core/player/strategy/TightAggressiveBotStrategy.h>
#include "StrategyTest.h"

using namespace pkt::core;

class TightAggressiveStrategyTest : public StrategyTest
{
  protected:
    TightAggressiveBotStrategy strategy;
};

TEST_F(TightAggressiveStrategyTest, PreflopRaisesWithPremiumHand)
{
    int raise = strategy.preflopShouldRaise(ctx, true);
    EXPECT_GT(raise, 0);
}

TEST_F(TightAggressiveStrategyTest, DoesNotRaiseWithTrashHand)
{
    ctx.myCard1 = "7d";
    ctx.myCard2 = "2c";

    int raise = strategy.preflopShouldRaise(ctx, true);
    EXPECT_EQ(raise, 0);
}
