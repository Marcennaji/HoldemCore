// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>
#include <string>

// Forward declarations from core interfaces
namespace pkt::core
{
class Logger;
class HandEvaluationEngine;
class PlayersStatisticsStore;
class Randomizer;
} // namespace pkt::core

namespace pkt::app
{

/**
 * @brief Types of logger implementations available
 */
enum class LoggerType
{
    Console, // Console output logger
    File,    // File-based logger (future)
    Null     // No-op logger for testing
};

/**
 * @brief Types of hand evaluation engines available
 */
enum class HandEvaluatorType
{
    Psim // Poker simulation engine (current default)
    // Future: PokerStove, Custom, etc.
};

/**
 * @brief Types of database implementations available
 */
enum class DatabaseType
{
    Sqlite, // SQLite database (current default)
    Null    // Null database for testing
    // Future: PostgreSql, MySQL, etc.
};

/**
 * @brief Factory for creating core application dependencies.
 *
 * This factory creates infrastructure dependencies without knowing about UI layers.
 * UI-specific factories (AppFactory, QmlAppFactory) use this to get their dependencies.
 */
class DependenciesFactory
{
  public:
    /**
     * @brief Creates a logger instance based on the specified type.
     */
    static std::shared_ptr<pkt::core::Logger> createLogger(LoggerType type);

    /**
     * @brief Creates a hand evaluation engine based on the specified type.
     */
    static std::shared_ptr<pkt::core::HandEvaluationEngine> createHandEvaluator(HandEvaluatorType type);

    /**
     * @brief Creates a statistics store based on the specified database type.
     */
    static std::shared_ptr<pkt::core::PlayersStatisticsStore> createStatisticsStore(DatabaseType type);

    /**
     * @brief Creates a randomizer instance.
     */
    static std::shared_ptr<pkt::core::Randomizer> createRandomizer();

    /**
     * @brief Gets the database file path for storage.
     */
    static std::string getDatabasePath();
};

} // namespace pkt::app
