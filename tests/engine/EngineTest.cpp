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
    services.setLogger(std::make_shared<pkt::infra::ConsoleLogger>());
    services.setHandEvaluationEngine(std::make_shared<pkt::infra::PsimHandEvaluationEngine>());
}

void EngineTest::TearDown()
{
}
// Helper function to initialize a Hand object for testing
void EngineTest::initializeHandForTesting(size_t activePlayerCount)
{
    myHand.reset();
    createPlayerList(activePlayerCount);
    initializeHandWithPlayers(activePlayerCount);
}

void EngineTest::createPlayerList(size_t playerCount)
{
    myPlayerList = std::make_shared<std::list<std::shared_ptr<Player>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        auto player = std::make_shared<DummyPlayer>(i, myEvents);
        player->setAction(PlayerActionNone);
        myPlayerList->push_back(player);
    }
}
void EngineTest::initializeHandWithPlayers(size_t activePlayerCount)
{
    myBoard = myFactory->createBoard(startDealerPlayerId);
    myBoard->setSeatsList(myPlayerList);
    myBoard->setRunningPlayersList(myPlayerList);

    GameData gameData;
    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;

    StartData startData;
    startData.startDealerPlayerId = startDealerPlayerId;
    startData.numberOfPlayers = static_cast<int>(activePlayerCount);

    myHand =
        myFactory->createHand(myFactory, myBoard, myPlayerList, myPlayerList, startDealerPlayerId, gameData, startData);
}

} // namespace pkt::test