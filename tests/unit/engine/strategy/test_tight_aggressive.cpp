#include "common/common.h"

#include <core/player/strategy/TightAggressiveBotStrategy.h>
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
    ctx.commonContext.gameState = Preflop;
    ctx.personalContext.holeCards = pkt::core::HoleCards("Ah", "Ad");

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Raise);
    EXPECT_GT(action.amount, 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_SuitedBroadway_Raises)
{
    ctx.commonContext.gameState = Preflop;
    ctx.personalContext.holeCards = pkt::core::HoleCards("Ks", "Qs");

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Raise);
    EXPECT_GT(action.amount, 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_Trash_DoesNotRaise)
{
    ctx.commonContext.gameState = Preflop;
    ctx.personalContext.holeCards = pkt::core::HoleCards("7d", "2c");

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Fold);
    EXPECT_EQ(action.amount, 0);
}

TEST_F(TightAggressiveStrategyTest, Preflop_InPosition_CallsWithOdds)
{
    ctx.commonContext.gameState = Preflop;
    ctx.personalContext.holeCards = pkt::core::HoleCards("8h", "9h");
    ctx.personalContext.position = PlayerPosition::Button;
    ctx.personalContext.m_handSimulation.winSd = 0.6f;
    ctx.commonContext.bettingContext.preflopRaisesNumber = 1;
    ctx.commonContext.playersContext.preflopLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);
    ctx.personalContext.hasPosition = true;

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Call);
    EXPECT_EQ(action.amount, 0);
}

// --- Flop ---

TEST_F(TightAggressiveStrategyTest, Flop_HighEquity_Raises)
{
    ctx.commonContext.gameState = Flop;
    ctx.personalContext.holeCards = pkt::core::HoleCards("Jh", "Js");
    ctx.commonContext.stringBoard = "2h 3d 7c";
    ctx.personalContext.position = PlayerPosition::Button;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    ctx.commonContext.playersContext.flopLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);
    ctx.personalContext.hasPosition = true;

    ctx.personalContext.m_handSimulation.win = 0.92f;
    ctx.personalContext.m_handSimulation.winSd = 0.95f;
    ctx.personalContext.m_handSimulation.winRanged = 0.9f;

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Raise);
    EXPECT_GT(action.amount, 0);
}

TEST_F(TightAggressiveStrategyTest, Flop_LowEquity_Folds)
{
    ctx.commonContext.gameState = Flop;
    ctx.personalContext.m_handSimulation.winSd = 0.2f;
    ctx.commonContext.bettingContext.potOdd = 10;
    ctx.personalContext.position = PlayerPosition::Button;
    ctx.commonContext.bettingContext.flopBetsOrRaisesNumber = 1;
    ctx.commonContext.playersContext.flopLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);
    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Fold);
    EXPECT_EQ(action.amount, 0);
}

// --- Turn ---

TEST_F(TightAggressiveStrategyTest, Turn_HighEquity_Raises)
{
    ctx.commonContext.gameState = Turn;
    ctx.personalContext.m_handSimulation.win = 0.95f;
    ctx.personalContext.m_handSimulation.winRanged = 0.95f;
    ctx.personalContext.position = PlayerPosition::Button;
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.playersContext.turnLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Raise);
    EXPECT_GT(action.amount, 0);
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_DoesNotRaise)
{
    ctx.commonContext.gameState = Turn;
    ctx.personalContext.m_handSimulation.win = 0.4f;
    ctx.personalContext.m_handSimulation.winRanged = 0.4f;
    ctx.personalContext.position = PlayerPosition::Button;
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.playersContext.turnLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_NE(action.type, ActionType::Raise);
    EXPECT_EQ(action.amount, 0);
}

TEST_F(TightAggressiveStrategyTest, Turn_LowEquity_Folds)
{
    ctx.commonContext.gameState = Turn;
    ctx.personalContext.m_handSimulation.winSd = 0.1f;
    ctx.commonContext.bettingContext.potOdd = 10;
    ctx.personalContext.position = PlayerPosition::Button;
    ctx.commonContext.bettingContext.turnBetsOrRaisesNumber = 1;
    ctx.commonContext.playersContext.turnLastRaiser = std::make_shared<pkt::test::DummyPlayer>(2, events);

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Fold);
    EXPECT_EQ(action.amount, 0);
}

// --- River ---

TEST_F(TightAggressiveStrategyTest, River_Nuts_Raises)
{
    ctx.commonContext.gameState = River;
    ctx.personalContext.m_handSimulation.winSd = 1.0f;
    ctx.personalContext.m_handSimulation.win = 1.0f;
    ctx.personalContext.m_handSimulation.winRanged = 1.0f;
    ctx.personalContext.position = PlayerPosition::Button;
    ctx.commonContext.bettingContext.riverBetsOrRaisesNumber = 5;
    ctx.commonContext.playersContext.lastVPIPPlayer = std::make_shared<pkt::test::DummyPlayer>(2, events);

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Raise);
    EXPECT_GT(action.amount, 0);
}

// --- Edge / Defensive ---

TEST_F(TightAggressiveStrategyTest, ZeroCash_CannotRaise)
{
    ctx.commonContext.gameState = Preflop;
    ctx.personalContext.m_handSimulation.winSd = 0.5f;
    ctx.personalContext.cash = 0;
    ctx.personalContext.holeCards = pkt::core::HoleCards("8h", "9h");

    PlayerAction action = strategy.decideAction(ctx);

    EXPECT_EQ(action.type, ActionType::Fold);
    EXPECT_EQ(action.amount, 0);
}

} // namespace pkt::test
