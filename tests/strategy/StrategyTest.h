#pragma once
#include <core/player/strategy/CurrentHandContext.h>
#include <gtest/gtest.h>
#include "DummyPlayer.h"

class StrategyTest : public ::testing::Test
{
  protected:
    pkt::core::CurrentHandContext ctx;
    std::shared_ptr<pkt::test::DummyPlayer> dummyPlayer;

    void SetUp() override
    {
        dummyPlayer = std::make_shared<pkt::test::DummyPlayer>(42);

        ctx.nbPlayers = 6;
        ctx.nbRunningPlayers = 6;
        ctx.pot = 100;
        ctx.highestSet = 20;
        ctx.myCash = 1000;
        ctx.mySet = 0;
        ctx.myCard1 = "Ah";
        ctx.myCard2 = "Ad";
        ctx.myHandSimulation.win = 0.8f;
        ctx.myHavePosition = true;
        ctx.smallBlind = 10;
        ctx.myID = dummyPlayer->getID();

        ctx.preflopLastRaiser = dummyPlayer;
        ctx.flopLastRaiser = dummyPlayer;
        ctx.turnLastRaiser = dummyPlayer;
        ctx.lastVPIPPlayer = dummyPlayer;
    }
};
