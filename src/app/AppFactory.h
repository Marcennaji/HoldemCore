// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <memory>
#include <string>

// Forward declarations from core interfaces
namespace pkt::core {
    class Logger;
    class HandEvaluationEngine;
    class PlayersStatisticsStore;
    class Randomizer;
}

namespace pkt::ui::qtwidgets {
    class GuiAppController;
}

namespace pkt::app {

/**
 * @brief Types of logger implementations available
 */
enum class LoggerType {
    Console,    // Console output logger
    File,       // File-based logger (future)
    Null        // No-op logger for testing
};

/**
 * @brief Types of hand evaluation engines available
 */
enum class HandEvaluatorType {
    Psim        // Poker simulation engine (current default)
    // Future: PokerStove, Custom, etc.
};

/**
 * @brief Types of database implementations available
 */
enum class DatabaseType {
    Sqlite,     // SQLite database (current default)
    Null        // Null database for testing
    // Future: PostgreSql, MySQL, etc.
};

/**
 * @brief Factory for creating application dependencies and main controller.
 * 
 * This factory serves as the composition root for the application, handling
 * all concrete dependency instantiation and wiring. 
 */
class AppFactory {
public:
    /**
     * @brief Creates a fully configured application controller.
     * 
     * @param loggerType Type of logger implementation to use
     * @param evaluatorType Type of hand evaluator implementation to use  
     * @param dbType Type of database implementation to use
     * @return Unique pointer to configured GuiAppController
     */
    static std::unique_ptr<pkt::ui::qtwidgets::GuiAppController> 
        createApplication(
            LoggerType loggerType = LoggerType::Console,
            HandEvaluatorType evaluatorType = HandEvaluatorType::Psim,
            DatabaseType dbType = DatabaseType::Sqlite
        );
    
    /**
     * @brief Creates application optimized for testing.
     * @return Unique pointer to configured GuiAppController with null implementations
     */
    static std::unique_ptr<pkt::ui::qtwidgets::GuiAppController> 
        createTestApp();

private:
    /**
     * @brief Creates a logger instance based on the specified type.
     */
    static std::shared_ptr<pkt::core::Logger> 
        createLogger(LoggerType type);
    
    /**
     * @brief Creates a hand evaluation engine based on the specified type.
     */
    static std::shared_ptr<pkt::core::HandEvaluationEngine> 
        createHandEvaluator(HandEvaluatorType type);
    
    /**
     * @brief Creates a statistics store based on the specified database type.
     */
    static std::shared_ptr<pkt::core::PlayersStatisticsStore> 
        createStatisticsStore(DatabaseType type);
    
    /**
     * @brief Creates a randomizer instance.
     */
    static std::shared_ptr<pkt::core::Randomizer> 
        createRandomizer();
    
    /**
     * @brief Gets the database file path for storage.
     */
    static std::string getDatabasePath();
};

} // namespace pkt::app