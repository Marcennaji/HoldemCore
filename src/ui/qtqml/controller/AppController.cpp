// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "AppController.h"
#include "Bridge.h"
#include "TableViewModel.h"

#include <core/engine/EngineFactory.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/engine/model/TableProfile.h>
#include <core/session/Session.h>

namespace pkt::ui::qtqml::controller
{

AppController::AppController(std::shared_ptr<pkt::core::Logger> logger,
                                   std::shared_ptr<pkt::core::HandEvaluationEngine> handEvaluationEngine,
                                   std::shared_ptr<pkt::core::PlayersStatisticsStore> statisticsStore,
                                   std::shared_ptr<pkt::core::Randomizer> randomizer, QObject* parent)
    : QObject(parent), m_logger(std::move(logger)), m_handEvaluationEngine(std::move(handEvaluationEngine)),
      m_statisticsStore(std::move(statisticsStore)), m_randomizer(std::move(randomizer))
{
    // Initialize core components
    m_engineFactory = std::make_shared<pkt::core::EngineFactory>(m_events, *m_logger, *m_handEvaluationEngine,
                                                                 *m_statisticsStore, *m_randomizer);

    m_session = std::make_unique<pkt::core::Session>(m_events, *m_engineFactory, *m_logger, *m_handEvaluationEngine,
                                                     *m_statisticsStore, *m_randomizer);

    // Create view model for QML binding
    m_tableViewModel = std::make_unique<TableViewModel>(this);

    // Create bridge to connect engine events to view model
    m_bridge = std::make_unique<Bridge>(m_session.get(), m_tableViewModel.get(), this);
    m_bridge->connectEventsToViewModel(m_events);
}

AppController::~AppController() = default;

void AppController::startGame(int playerCount, int smallBlind, int bigBlind, const QString& botProfile,
                                 double gameSpeed)
{
    // Simple logging without format strings
    m_logger->info("Starting new game from QML");

    // Convert bot profile QString to TableProfile enum
    pkt::core::TableProfile tableProfile = pkt::core::TableProfile::RandomOpponents;
    if (botProfile == "Loose")
    {
        tableProfile = pkt::core::TableProfile::LargeAgressiveOpponents;
    }
    else if (botProfile == "Tight")
    {
        tableProfile = pkt::core::TableProfile::TightAgressiveOpponents;
    }
    else if (botProfile == "Aggressive")
    {
        tableProfile = pkt::core::TableProfile::LargeAgressiveOpponents;
    }

    // Create GameData
    pkt::core::GameData gameData;
    gameData.maxNumberOfPlayers = playerCount;
    gameData.startMoney = 5000; // Default starting money
    gameData.firstSmallBlind = smallBlind;
    gameData.tableProfile = tableProfile;
    gameData.guiSpeed = static_cast<int>(gameSpeed);

    // Create StartData
    pkt::core::StartData startData;
    startData.numberOfPlayers = playerCount;
    // Dealer will be auto-selected by Session (StartData::AUTO_SELECT_DEALER is the default)

    // Start the game session
    m_session->startGame(gameData, startData);

    // Notify view model that game has started
    m_tableViewModel->setGameActive(true);
}

void AppController::exitToSetup()
{
    m_logger->info("Exiting to setup screen");

    // Reset view model (game will naturally end when window closes)
    m_tableViewModel->reset();
    m_tableViewModel->setGameActive(false);
}

} // namespace pkt::ui::qtqml::controller
