#pragma once
#include <core/player/strategy/CurrentHandContext.h>
#include <gtest/gtest.h>
#include "DummyPlayer.h"

class StrategyTest : public ::testing::Test
{
  protected:
    pkt::core::CurrentHandContext ctx;

    void SetUp() override
    {
        ctx.reset();
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
