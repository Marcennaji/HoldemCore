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
        ctx.commonContext.nbPlayers = 6;
        ctx.commonContext.nbRunningPlayers = 6;
        ctx.commonContext.pot = 100;
        ctx.commonContext.highestSet = 20;
        ctx.perPlayerContext.myCash = 1000;
        ctx.perPlayerContext.mySet = 0;
        ctx.perPlayerContext.myID = 1;
        ctx.commonContext.smallBlind = 10;
    }
};

} // namespace pkt::test
