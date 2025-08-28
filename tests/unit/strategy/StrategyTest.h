#pragma once
#include "common/common.h"

#include <core/player/strategy/CurrentHandContext.h>
#include "common/DummyPlayer.h"
#include "common/EngineTest.h"

namespace pkt::test
{
class StrategyTest : public EngineTest
{
  protected:
    pkt::core::player::CurrentHandContext ctx;

    void SetUp() override
    {
        EngineTest::SetUp();

        initializeHandForTesting(6, gameData);
        ctx.commonContext.playersContext.nbPlayers = 6;
        ctx.commonContext.playersContext.runningPlayersList = myRunningPlayersList;
        ctx.commonContext.bettingContext.pot = 100;
        ctx.commonContext.bettingContext.highestBetAmount = 20;
        ctx.personalContext.cash = 1000;
        ctx.personalContext.totalBetAmount = 0;
        ctx.personalContext.id = 1;
        ctx.commonContext.smallBlind = 10;
    }
};

} // namespace pkt::test
