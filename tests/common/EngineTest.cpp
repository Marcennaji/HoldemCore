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
    logger->setLogLevel(pkt::core::LogLevel::Info);
    services.setLogger(std::move(logger));
    services.setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>());
    auto randomizer = std::make_unique<FakeRandomizer>();
    randomizer->values = {3, 5, 7};
    services.setRandomizer(std::move(randomizer));

    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;
}

void EngineTest::TearDown()
{
}

void EngineTest::createPlayersFsmLists(size_t playerCount)
{
    mySeatsListFsm = std::make_shared<std::list<std::shared_ptr<PlayerFsm>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        auto playerFsm = std::make_shared<DummyPlayerFsm>(i, myEvents);
        mySeatsListFsm->push_back(playerFsm);
    }

    myActingPlayersListFsm = std::make_shared<std::list<std::shared_ptr<PlayerFsm>>>();
    for (const auto& player : *mySeatsListFsm)
    {
        myActingPlayersListFsm->push_back(player);
    }
}
void EngineTest::initializeHandFsmWithPlayers(size_t activePlayerCount, GameData gameData)
{
    createPlayersFsmLists(activePlayerCount);
    myBoardFsm = myFactory->createBoardFsm(startDealerPlayerId);
    myBoardFsm->setSeatsListFsm(mySeatsListFsm);
    myBoardFsm->setActingPlayersListFsm(myActingPlayersListFsm);

    StartData startData;
    startData.startDealerPlayerId = startDealerPlayerId;
    startData.numberOfPlayers = static_cast<int>(activePlayerCount);

    myHandFsm =
        myFactory->createHandFsm(myFactory, myBoardFsm, mySeatsListFsm, myActingPlayersListFsm, gameData, startData);

    myHandFsm->initialize();
}
void EngineTest::checkPostRiverConditions()
{

    EXPECT_EQ(myHandFsm->getGameState(), PostRiver);
    EXPECT_EQ(myHandFsm->getBoard().getPot(*myHandFsm), 0); // Pot should be reset to 0
}

bool EngineTest::isPlayerStillActive(unsigned id) const
{
    for (const auto& p : *myHandFsm->getActingPlayersList())
    {
        if (p->getId() == id)
            return true;
    }
    return false;
}

} // namespace pkt::test