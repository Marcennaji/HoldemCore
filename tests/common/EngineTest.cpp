#include "common/EngineTest.h"
#include "FakeRandomizer.h"
#include "PlayerFsm.h"
#include "common/DummyPlayer.h"
#include "core/engine/Exception.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/services/GlobalServices.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void EngineTest::SetUp()
{
    myFactory = std::make_unique<EngineFactory>(myEvents);
    auto& services = pkt::core::GlobalServices::instance();
    auto logger = std::make_unique<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(pkt::core::LogLevel::Quiet);
    services.setLogger(std::move(logger));
    services.setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>());
    auto randomizer = std::make_unique<FakeRandomizer>();
    randomizer->values = {3, 5, 7};
    services.setRandomizer(std::move(randomizer));
}

void EngineTest::TearDown()
{
}
// Helper function to initialize a Hand object for testing
void EngineTest::initializeHandForTesting(size_t activePlayerCount, GameData gameData)
{
    myHand.reset();
    createPlayersLists(activePlayerCount);
    initializeHandWithPlayers(activePlayerCount, gameData);
}
void EngineTest::initializeHandFsmForTesting(size_t activePlayerCount, GameData gameData)
{
    myHandFsm.reset();
    createPlayersFsmLists(activePlayerCount);
    initializeHandFsmWithPlayers(activePlayerCount, gameData);
}
void EngineTest::createPlayersLists(size_t playerCount)
{
    mySeatsList = std::make_shared<std::list<std::shared_ptr<Player>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        auto player = std::make_shared<DummyPlayer>(i, myEvents);
        mySeatsList->push_back(player);
    }
    // Create a deep copy of mySeatsList for RunningPlayersList
    myRunningPlayersList = std::make_shared<std::list<std::shared_ptr<Player>>>();
    for (const auto& player : *mySeatsList)
    {
        myRunningPlayersList->push_back(player);
    }
}
void EngineTest::initializeHandWithPlayers(size_t activePlayerCount, GameData gameData)
{
    myBoard = myFactory->createBoard(startDealerPlayerId);
    myBoard->setSeatsList(mySeatsList);
    myBoard->setRunningPlayersList(myRunningPlayersList);

    StartData startData;
    startData.startDealerPlayerId = startDealerPlayerId;
    startData.numberOfPlayers = static_cast<int>(activePlayerCount);

    myHand = myFactory->createHand(myFactory, myBoard, mySeatsList, myRunningPlayersList, gameData, startData);
}
void EngineTest::createPlayersFsmLists(size_t playerCount)
{
    mySeatsListFsm = std::make_unique<std::list<std::shared_ptr<PlayerFsm>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        auto playerFsm = std::make_shared<DummyPlayerFsm>(i, myEvents);
        mySeatsListFsm->push_back(playerFsm);
    }

    myRunningPlayersListFsm = std::make_shared<std::list<std::shared_ptr<PlayerFsm>>>();
    for (const auto& player : *mySeatsListFsm)
    {
        myRunningPlayersListFsm->push_back(player);
    }
}
void EngineTest::initializeHandFsmWithPlayers(size_t activePlayerCount, GameData gameData)
{
    createPlayersFsmLists(activePlayerCount);
    myBoard = myFactory->createBoard(startDealerPlayerId);
    myBoard->setSeatsListFsm(mySeatsListFsm);
    myBoard->setRunningPlayersListFsm(myRunningPlayersListFsm);

    StartData startData;
    startData.startDealerPlayerId = startDealerPlayerId;
    startData.numberOfPlayers = static_cast<int>(activePlayerCount);

    myHandFsm =
        myFactory->createHandFsm(myFactory, myBoard, mySeatsListFsm, myRunningPlayersListFsm, gameData, startData);
}
void EngineTest::checkPostRiverConditions()
{
    // Check if the hand has been resolved correctly after the river
    EXPECT_EQ(myHand->getCurrentRoundState(), PostRiver);
    EXPECT_EQ(myHand->getBoard()->getPot(), 0); // Pot should be reset to 0
}

} // namespace pkt::test