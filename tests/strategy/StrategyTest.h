#pragma once
#include <core/player/strategy/CurrentHandContext.h>
#include <gtest/gtest.h>
#include "DummyPlayer.h"

namespace pkt::test
{
class StrategyTest : public ::testing::Test
{
  protected:
    pkt::core::player::CurrentHandContext ctx;

    void SetUp() override
    {
        ctx.nbPlayers = 6;
        ctx.nbRunningPlayers = 6;
        ctx.pot = 100;
        ctx.highestSet = 20;
        ctx.myCash = 1000;
        ctx.mySet = 0;
        ctx.myID = 1;
        ctx.smallBlind = 10;
    }
};

} // namespace pkt::test
