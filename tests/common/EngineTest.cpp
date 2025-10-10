#include "common/EngineTest.h"
#include "FakeRandomizer.h"
#include "Player.h"
#include "common/DummyPlayer.h"
#include "core/engine/Exception.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "infra/persistence/NullPlayersStatisticsStore.h"
#include "core/player/Player.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <crtdbg.h>
#endif

using namespace pkt::core;
using namespace pkt::core::player;

namespace pkt::test
{

void EngineTest::SetUp()
{
#ifdef _DEBUG
    // In debug mode, disable gtest exception catching to get proper stack traces
    testing::GTEST_FLAG(catch_exceptions) = false;
    
    // Windows-specific: Disable error dialog boxes, let debugger handle crashes
    #ifdef _WIN32
    SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    #endif
#endif

    auto logger = std::make_unique<pkt::infra::ConsoleLogger>();
    logger->setLogLevel(getTestLogLevel()); 
    m_logger = std::move(logger);
    
    m_handEvaluationEngine = std::make_unique<pkt::infra::PsimHandEvaluationEngine>();
    
    auto randomizer = std::make_unique<FakeRandomizer>();
    randomizer->values = {3, 5, 7};
    m_randomizer = std::move(randomizer);
    
    // By default, use NullPlayersStatisticsStore
    m_playersStatisticsStore = std::make_shared<pkt::infra::NullPlayersStatisticsStore>();

    m_factory = std::make_unique<EngineFactory>(m_events, *m_logger, *m_handEvaluationEngine, *m_playersStatisticsStore, *m_randomizer);

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
        auto player = std::make_shared<DummyPlayer>(i, m_events, m_logger, m_handEvaluationEngine, m_playersStatisticsStore, m_randomizer);
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

    m_hand = m_factory->createHand(m_board, m_seatsList, m_actingPlayersList, gameData, startData);

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
    return *m_logger;
}

std::shared_ptr<pkt::core::Logger> EngineTest::getLoggerService() const
{
    return m_logger;
}

std::shared_ptr<pkt::core::HandEvaluationEngine> EngineTest::getHandEvaluationEngineService() const
{
    return m_handEvaluationEngine;
}

std::shared_ptr<pkt::core::PlayersStatisticsStore> EngineTest::getPlayersStatisticsStoreService() const
{
    return m_playersStatisticsStore;
}

std::shared_ptr<pkt::core::Randomizer> EngineTest::getRandomizerService() const
{
    return m_randomizer;
}

} // namespace pkt::test