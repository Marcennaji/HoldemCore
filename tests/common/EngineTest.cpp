#include "common/EngineTest.h"
#include "FakeRandomizer.h"
#include "Player.h"
#include "common/DummyPlayer.h"
#include "core/engine/Exception.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/interfaces/ServiceAdapter.h"
#include "core/player/Player.h"
#include "core/services/ServiceContainer.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

#include <memory>

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void EngineTest::SetUp()
{
    m_services = std::make_shared<pkt::core::AppServiceContainer>();
    auto logger = std::make_unique<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(pkt::core::LogLevel::Info);
    m_services->setLogger(std::move(logger));
    m_services->setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>());
    auto randomizer = std::make_unique<FakeRandomizer>();
    randomizer->values = {3, 5, 7};
    m_services->setRandomizer(std::move(randomizer));

    // Create ISP-compliant interfaces for the factory
    auto serviceAdapter = std::make_shared<pkt::core::ServiceAdapter>(m_services);
    auto loggerInterface = serviceAdapter->createLoggerService();
    auto handEvaluatorInterface = serviceAdapter->createHandEvaluationEngineService();
    auto statisticsStoreInterface = serviceAdapter->createPlayersStatisticsStoreService();

    // Use ISP-compliant constructor with all required focused interfaces
    m_factory = std::make_unique<EngineFactory>(m_events, loggerInterface, handEvaluatorInterface, statisticsStoreInterface);

    gameData.maxNumberOfPlayers = MAX_NUMBER_OF_PLAYERS;
    gameData.startMoney = 1000;
    gameData.firstSmallBlind = 10;
    gameData.tableProfile = TableProfile::RandomOpponents;
}

void EngineTest::TearDown()
{
}

void EngineTest::createPlayersLists(size_t playerCount)
{
    m_seatsList = std::make_shared<std::list<std::shared_ptr<pkt::core::player::Player>>>();
    for (size_t i = 0; i < playerCount; ++i)
    {
        auto player = std::make_shared<DummyPlayer>(i, m_events, m_services);
        m_seatsList->push_back(player);
    }

    m_actingPlayersList = std::make_shared<std::list<std::shared_ptr<pkt::core::player::Player>>>();
    for (const auto& player : *m_seatsList)
    {
        m_actingPlayersList->push_back(player);
    }
}
void EngineTest::initializeHandWithPlayers(size_t activePlayerCount, GameData gameData)
{
    createPlayersLists(activePlayerCount);
    m_board = m_factory->createBoard(startDealerPlayerId);
    m_board->setSeatsList(m_seatsList);
    m_board->setActingPlayersList(m_actingPlayersList);

    StartData startData;
    startData.startDealerPlayerId = startDealerPlayerId;
    startData.numberOfPlayers = static_cast<int>(activePlayerCount);

    m_hand = m_factory->createHand(m_factory, m_board, m_seatsList, m_actingPlayersList, gameData, startData);

    m_hand->initialize();
}
void EngineTest::checkPostRiverConditions()
{

    EXPECT_EQ(m_hand->getGameState(), PostRiver);
    EXPECT_EQ(m_hand->getBoard().getPot(*m_hand), 0); // Pot should be reset to 0
}

bool EngineTest::isPlayerStillActive(unsigned id) const
{
    for (const auto& p : *m_hand->getActingPlayersList())
    {
        if (p->getId() == id)
            return true;
    }
    return false;
}

pkt::core::Logger& EngineTest::getLogger() const
{
    return m_services->logger();
}

std::shared_ptr<pkt::core::ServiceContainer> EngineTest::getServices() const
{
    return m_services;
}

} // namespace pkt::test