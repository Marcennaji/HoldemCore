#include "EngineTest.h"
#include "DummyPlayer.h"
#include "core/engine/Exception.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/services/GlobalServices.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

#include <gtest/gtest.h>
#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void EngineTest::SetUp()
{
    myFactory = std::make_shared<EngineFactory>(myEvents);
    auto& services = pkt::core::GlobalServices::instance();
    auto logger = std::make_shared<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(pkt::core::LogLevel::Quiet);
    services.setLogger(logger);
    services.setHandEvaluationEngine(std::make_shared<pkt::infra::PsimHandEvaluationEngine>());
}

void EngineTest::TearDown()
{
}
// Helper function to initialize a Hand object for testing
void EngineTest::initializeHandForTesting(size_t activePlayerCount)
{
    myHand.reset();
    createPlayersLists(activePlayerCount);
    initializeHandWithPlayers(activePlayerCount);
}

void EngineTest::createPlayersLists(size_t playerCount)
{
    mySeatsList = std::make_shared<std::list<std::shared_ptr<Player>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        auto player = std::make_shared<DummyPlayer>(i, myEvents);
        player->setAction(PlayerActionNone);
        mySeatsList->push_back(player);
    }
    // Create a deep copy of mySeatsList for RunningPlayersList
    myRunningPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>();
    for (const auto& player : *mySeatsList)
    {
        myRunningPlayersList->push_back(player);
    }
}
void EngineTest::initializeHandWithPlayers(size_t activePlayerCount)
{
    myBoard = myFactory->createBoard(startDealerPlayerId);
    myBoard->setSeatsList(mySeatsList);
    myBoard->setRunningPlayersList(myRunningPlayersList);

    GameData gameData;
    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;

    StartData startData;
    startData.startDealerPlayerId = startDealerPlayerId;
    startData.numberOfPlayers = static_cast<int>(activePlayerCount);

    myHand = myFactory->createHand(myFactory, myBoard, mySeatsList, myRunningPlayersList, startDealerPlayerId, gameData,
                                   startData);
    myHand->setFlowMode(myFlowMode);
}

void EngineTest::checkPostRiverConditions()
{
    // Check if the hand has been resolved correctly after the river
    EXPECT_EQ(myHand->getCurrentRoundState(), GameStatePostRiver);
    EXPECT_EQ(myHand->getBoard()->getPot(), 0); // Pot should be reset to 0
    EXPECT_FALSE(myHand->getBoard()->getWinners().empty());
}

} // namespace pkt::test