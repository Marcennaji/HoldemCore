#include "QmlAppController.h"

#include <app/Session.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/StartData.h>
#include <core/engine/model/TableProfile.h>
#include <infra/ConsoleLogger.h>
#include <infra/persistence/SqliteLogStore.h>
#include "core/engine/Randomizer.h"

#include <QDebug>

QmlAppController::QmlAppController(QObject* parent) : QObject(parent)
{
    setupBackend();
}

QmlAppController::~QmlAppController() = default;

void QmlAppController::setupBackend()
{
    gameActionslogger = std::make_unique<SqliteLogStore>("mylog.db");
    auto logger = std::make_unique<ConsoleLogger>();

    mySession = std::make_unique<Session>(logger.get(), nullptr, gameActionslogger.get(), gameActionslogger.get(),
                                          gameActionslogger.get());
    mySession->init();
}

void QmlAppController::startGame(int playerCount, int startChips, const QString& profileStr)
{
    GameData gameData;

    gameData.maxNumberOfPlayers = playerCount;
    gameData.startMoney = startChips;
    gameData.firstSmallBlind = 20; // default small blind
    gameData.guiSpeed = 8;

    if (profileStr == "LARGE_AGRESSIVE_OPPONENTS")
        gameData.tableProfile = TableProfile::LARGE_AGRESSIVE_OPPONENTS;
    else if (profileStr == "TIGHT_AGRESSIVE_OPPONENTS")
        gameData.tableProfile = TableProfile::TIGHT_AGRESSIVE_OPPONENTS;
    else
        gameData.tableProfile = TableProfile::RANDOM_OPPONENTS;

    gameData.guiSpeed = GAME_SPEED;

    StartData startData;
    int tmpDealerPos = 0;
    startData.numberOfPlayers = gameData.maxNumberOfPlayers;

    Randomizer::GetRand(0, startData.numberOfPlayers - 1, 1, &tmpDealerPos);
    startData.startDealerPlayerId = static_cast<unsigned>(tmpDealerPos);

    mySession->startGame(gameData, startData);
    emit gameStarted();
}

void QmlAppController::resetGame()
{
    mySession.reset();
    setupBackend();
    emit gameEnded();
}
