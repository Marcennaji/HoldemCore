// HoldemCore — Bot Statistics Generator Tool
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Core engine
#include "core/engine/EngineFactory.h"
#include "core/engine/model/BotGameData.h"
#include "core/engine/model/GameData.h"
#include "core/engine/model/StartData.h"
#include "core/engine/utils/ExceptionUtils.h"
#include "core/session/Session.h"

// Infrastructure
#include "adapters/infrastructure/hand_evaluation/PsimHandEvaluationEngine.h"
#include "adapters/infrastructure/logger/ConsoleLogger.h"
#include "adapters/infrastructure/logger/NullLogger.h"
#include "adapters/infrastructure/randomizer/DefaultRandomizer.h"
#include "adapters/infrastructure/statistics/sqlite/SqliteDb.h"
#include "adapters/infrastructure/statistics/sqlite/SqlitePlayersStatisticsStore.h"

using namespace pkt::core;
using namespace pkt::infra;

// Structure to hold strategy configuration
struct StrategyConfig
{
    std::string name;
    int count;
};

// Structure to hold statistics for reporting
struct CalculatedStats
{
    std::string strategyName;
    std::string tableType;

    // Raw data
    int pf_hands = 0;
    int pf_checks = 0;
    int pf_calls = 0;
    int pf_raises = 0;
    int pf_threeBets = 0;
    int pf_fourBets = 0;
    int pf_folds = 0;
    int pf_limps = 0;
    int pf_callThreeBets = 0;
    int pf_callThreeBetsOpportunities = 0;

    int f_hands = 0;
    int f_checks = 0;
    int f_bets = 0;
    int f_calls = 0;
    int f_raises = 0;
    int f_folds = 0;
    int f_continuationBets = 0;
    int f_continuationBetsOpportunities = 0;

    int t_hands = 0;
    int t_checks = 0;
    int t_bets = 0;
    int t_calls = 0;
    int t_raises = 0;
    int t_folds = 0;

    int r_hands = 0;
    int r_checks = 0;
    int r_bets = 0;
    int r_calls = 0;
    int r_raises = 0;
    int r_folds = 0;

    // Calculated percentages
    double vpip = 0.0;
    double pfr = 0.0;
    double threeBetPct = 0.0;
    double callThreeBetPct = 0.0;
    double limpPct = 0.0;
    double foldPreflopPct = 0.0;
    double flopCBetPct = 0.0;
    double flopFoldPct = 0.0;
    double turnAggressionPct = 0.0;
    double riverAggressionPct = 0.0;
    double aggressionFactor = 0.0;
    double aggressionFrequency = 0.0;
};

void printUsage(const char* programName)
{
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  -n, --hands <number>           Number of hands to simulate (default: 1000)\n"
              << "  -s, --strategy <name:count>    Add strategy with count (REQUIRED, can be used multiple times)\n"
              << "                                 Available: tight, loose, maniac, ultratight\n"
              << "                                 Example: -s tight:2 -s loose:2\n"
              << "  -m, --money <amount>           Initial cash amount for each bot (default: 3000)\n"
              << "  -l, --logger <type>            Logger type: console, quiet, null (default: quiet)\n"
              << "  -o, --output <file>            Output report file (default: bot_statistics_report.txt)\n"
              << "  -d, --database <file>          Database file path (default: bot_statistics.db)\n"
              << "  -h, --help                     Show this help message\n\n"
              << "Examples:\n"
              << "  " << programName << " -n 10000 -s tight:2 -s loose:2 -s maniac:1 -s ultratight:1\n"
              << "  " << programName << " -n 5000 -s tight:3 -s loose:3 -m 5000 -l console\n";
}

std::string getDefaultDatabasePath()
{
    return "bot_statistics.db";
}

// Structure to hold strategy configuration for custom strategy mix
struct StrategySpec
{
    std::string name;
    int count;
};

bool parseStrategyArg(const std::string& arg, StrategySpec& spec)
{
    size_t colonPos = arg.find(':');
    if (colonPos == std::string::npos)
    {
        return false;
    }

    spec.name = arg.substr(0, colonPos);
    try
    {
        spec.count = std::stoi(arg.substr(colonPos + 1));
        if (spec.count <= 0)
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }

    // Validate strategy name
    std::string lowerName = spec.name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    if (lowerName != "tight" && lowerName != "loose" && lowerName != "maniac" && lowerName != "ultratight")
    {
        return false;
    }

    return true;
}

CalculatedStats calculateStatistics(const std::string& strategyName, const std::string& tableType, SqliteDb& db)
{
    CalculatedStats stats;
    stats.strategyName = strategyName;
    stats.tableType = tableType;

    auto stmt = db.prepare("SELECT "
                           "pf_hands, pf_checks, pf_calls, pf_raises, pf_threeBets, pf_fourBets, pf_folds, pf_limps, "
                           "pf_callThreeBets, pf_callThreeBetsOpportunities, "
                           "f_hands, f_checks, f_bets, f_calls, f_raises, f_folds, "
                           "f_continuationBets, f_continuationBetsOpportunities, "
                           "t_hands, t_checks, t_bets, t_calls, t_raises, t_folds, "
                           "r_hands, r_checks, r_bets, r_calls, r_raises, r_folds "
                           "FROM PlayersStatistics WHERE strategy_name=?1 AND table_type=?2");

    stmt->bindText(1, strategyName);
    stmt->bindText(2, tableType);

    if (!stmt->step())
    {
        return stats; // No data found
    }

    // Load raw data
    stats.pf_hands = stmt->getInt(0);
    stats.pf_checks = stmt->getInt(1);
    stats.pf_calls = stmt->getInt(2);
    stats.pf_raises = stmt->getInt(3);
    stats.pf_threeBets = stmt->getInt(4);
    stats.pf_fourBets = stmt->getInt(5);
    stats.pf_folds = stmt->getInt(6);
    stats.pf_limps = stmt->getInt(7);
    stats.pf_callThreeBets = stmt->getInt(8);
    stats.pf_callThreeBetsOpportunities = stmt->getInt(9);

    stats.f_hands = stmt->getInt(10);
    stats.f_checks = stmt->getInt(11);
    stats.f_bets = stmt->getInt(12);
    stats.f_calls = stmt->getInt(13);
    stats.f_raises = stmt->getInt(14);
    stats.f_folds = stmt->getInt(15);
    stats.f_continuationBets = stmt->getInt(16);
    stats.f_continuationBetsOpportunities = stmt->getInt(17);

    stats.t_hands = stmt->getInt(18);
    stats.t_checks = stmt->getInt(19);
    stats.t_bets = stmt->getInt(20);
    stats.t_calls = stmt->getInt(21);
    stats.t_raises = stmt->getInt(22);
    stats.t_folds = stmt->getInt(23);

    stats.r_hands = stmt->getInt(24);
    stats.r_checks = stmt->getInt(25);
    stats.r_bets = stmt->getInt(26);
    stats.r_calls = stmt->getInt(27);
    stats.r_raises = stmt->getInt(28);
    stats.r_folds = stmt->getInt(29);

    // Calculate percentages
    if (stats.pf_hands > 0)
    {
        stats.vpip = 100.0 *
                     (stats.pf_calls + stats.pf_raises + stats.pf_threeBets + stats.pf_fourBets + stats.pf_limps) /
                     stats.pf_hands;
        stats.pfr = 100.0 * (stats.pf_raises + stats.pf_threeBets + stats.pf_fourBets) / stats.pf_hands;
        stats.limpPct = 100.0 * stats.pf_limps / stats.pf_hands;
        stats.foldPreflopPct = 100.0 * stats.pf_folds / stats.pf_hands;
    }

    if (stats.pf_callThreeBetsOpportunities > 0)
    {
        stats.threeBetPct = 100.0 * stats.pf_threeBets / stats.pf_callThreeBetsOpportunities;
        stats.callThreeBetPct = 100.0 * stats.pf_callThreeBets / stats.pf_callThreeBetsOpportunities;
    }

    if (stats.f_continuationBetsOpportunities > 0)
    {
        stats.flopCBetPct = 100.0 * stats.f_continuationBets / stats.f_continuationBetsOpportunities;
    }

    if (stats.f_hands > 0)
    {
        stats.flopFoldPct = 100.0 * stats.f_folds / stats.f_hands;
    }

    int turnActions = stats.t_bets + stats.t_raises + stats.t_calls + stats.t_checks;
    if (turnActions > 0)
    {
        stats.turnAggressionPct = 100.0 * (stats.t_bets + stats.t_raises) / turnActions;
    }

    int riverActions = stats.r_bets + stats.r_raises + stats.r_calls + stats.r_checks;
    if (riverActions > 0)
    {
        stats.riverAggressionPct = 100.0 * (stats.r_bets + stats.r_raises) / riverActions;
    }

    int totalCalls = stats.pf_calls + stats.f_calls + stats.t_calls + stats.r_calls;
    int totalBets = stats.f_bets + stats.t_bets + stats.r_bets;
    int totalRaises =
        stats.pf_raises + stats.pf_threeBets + stats.pf_fourBets + stats.f_raises + stats.t_raises + stats.r_raises;

    if (totalCalls > 0)
    {
        stats.aggressionFactor = static_cast<double>(totalBets + totalRaises) / totalCalls;
    }
    else if (totalBets + totalRaises > 0)
    {
        stats.aggressionFactor = 999.99;
    }

    int totalChecks = stats.pf_checks + stats.f_checks + stats.t_checks + stats.r_checks;
    int allActions = totalBets + totalRaises + totalCalls + totalChecks;
    if (allActions > 0)
    {
        stats.aggressionFrequency = 100.0 * (totalBets + totalRaises) / allActions;
    }

    return stats;
}

void generateReport(const std::string& dbPath, const std::string& outputFile, int totalPlayers)
{
    std::cout << "\n=== Generating Statistics Report ===\n";

    SqliteDb db(dbPath);
    std::ofstream report(outputFile);

    if (!report.is_open())
    {
        pkt::core::utils::throwRuntimeError("Could not open output file: " + outputFile);
    }

    // Report header
    report << "========================================\n";
    report << "  BOT STRATEGIES STATISTICS REPORT\n";
    report << "========================================\n\n";

    // Determine table type
    std::string tableType;
    if (totalPlayers == 2)
        tableType = "HU";
    else if (totalPlayers <= 6)
        tableType = "SH";
    else
        tableType = "FR";

    report << "Table Type: " << tableType << " (" << totalPlayers << " players)\n\n";

    // Query all unique strategies from the database
    auto strategyStmt = db.prepare("SELECT DISTINCT strategy_name FROM PlayersStatistics WHERE table_type=?1");
    strategyStmt->bindText(1, tableType);

    std::vector<std::string> strategyNames;
    while (strategyStmt->step())
    {
        strategyNames.push_back(strategyStmt->getText(0));
    }

    // Generate statistics for each strategy
    for (const auto& strategyName : strategyNames)
    {
        CalculatedStats stats = calculateStatistics(strategyName, tableType, db);

        if (stats.pf_hands == 0)
        {
            continue;
        }

        report << "========================================\n";
        report << "Strategy: " << strategyName << "\n";
        report << "========================================\n\n";

        report << "PREFLOP STATISTICS:\n";
        report << std::fixed << std::setprecision(2);
        report << "  VPIP (Voluntarily Put $ In Pot):     " << std::setw(6) << stats.vpip << "%\n";
        report << "  PFR (Preflop Raise):                  " << std::setw(6) << stats.pfr << "%\n";
        report << "  3Bet%:                                " << std::setw(6) << stats.threeBetPct << "%\n";
        report << "  Call 3Bet%:                           " << std::setw(6) << stats.callThreeBetPct << "%\n";
        report << "  Limp%:                                " << std::setw(6) << stats.limpPct << "%\n";
        report << "  Fold Preflop%:                        " << std::setw(6) << stats.foldPreflopPct << "%\n";
        report << "  Total Hands:                          " << std::setw(6) << stats.pf_hands << "\n\n";

        report << "POSTFLOP STATISTICS:\n";
        report << "  Flop C-Bet%:                          " << std::setw(6) << stats.flopCBetPct << "%\n";
        report << "  Flop Fold%:                           " << std::setw(6) << stats.flopFoldPct << "%\n";
        report << "  Turn Aggression%:                     " << std::setw(6) << stats.turnAggressionPct << "%\n";
        report << "  River Aggression%:                    " << std::setw(6) << stats.riverAggressionPct << "%\n\n";

        report << "OVERALL AGGRESSION:\n";
        report << "  Aggression Factor (AF):               " << std::setw(6) << stats.aggressionFactor << "\n";
        report << "  Aggression Frequency (AFq):           " << std::setw(6) << stats.aggressionFrequency << "%\n\n";

        report << "RAW DATA:\n";
        report << "  Preflop: " << stats.pf_hands << " hands, " << stats.pf_raises << " raises, " << stats.pf_threeBets
               << " 3bets, " << stats.pf_calls << " calls, " << stats.pf_folds << " folds\n";
        report << "  Flop: " << stats.f_hands << " hands, " << stats.f_bets << " bets, " << stats.f_raises
               << " raises, " << stats.f_calls << " calls, " << stats.f_folds << " folds\n";
        report << "  Turn: " << stats.t_hands << " hands, " << stats.t_bets << " bets, " << stats.t_raises
               << " raises\n";
        report << "  River: " << stats.r_hands << " hands, " << stats.r_bets << " bets, " << stats.r_raises
               << " raises\n\n";
    }

    report << "========================================\n";
    report << "NOTE: The following statistics are not yet available:\n";
    report << "  - Went to Showdown (WtSD%)\n";
    report << "  - Won $ at Showdown (W$SD%)\n";
    report << "  - Won When Saw Flop (WWSF%)\n";
    report << "  - Average Bet Size Ratio\n";
    report << "========================================\n";

    report.close();

    std::cout << "Report saved to: " << outputFile << "\n";
}

// Configuration structure to hold parsed command-line arguments
struct SimulationConfig
{
    int numHands = 1000;
    int numPlayers = 6;
    int startMoney = 3000;
    std::vector<StrategySpec> customStrategies;
    std::string loggerType = "quiet";
    std::string outputFile = "bot_statistics_report.txt";
    std::string dbPath = getDefaultDatabasePath();
};

// Parse command-line arguments into configuration
SimulationConfig parseCommandLineArgs(int argc, char* argv[])
{
    SimulationConfig config;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            printUsage(argv[0]);
            exit(0);
        }
        else if (arg == "-n" || arg == "--hands")
        {
            if (i + 1 >= argc)
            {
                pkt::core::utils::throwRuntimeError("Error: " + arg + " requires an argument");
            }
            config.numHands = std::stoi(argv[++i]);
        }
        else if (arg == "-s" || arg == "--strategy")
        {
            if (i + 1 >= argc)
            {
                pkt::core::utils::throwRuntimeError("Error: " + arg + " requires an argument");
            }
            StrategySpec spec;
            if (!parseStrategyArg(argv[++i], spec))
            {
                pkt::core::utils::throwRuntimeError("Error: Invalid strategy format. Use: strategy:count\n"
                                                    "Available strategies: tight, loose, maniac, ultratight");
            }
            config.customStrategies.push_back(spec);
        }
        else if (arg == "-m" || arg == "--money")
        {
            if (i + 1 >= argc)
            {
                pkt::core::utils::throwRuntimeError("Error: " + arg + " requires an argument");
            }
            config.startMoney = std::stoi(argv[++i]);
        }
        else if (arg == "-l" || arg == "--logger")
        {
            if (i + 1 >= argc)
            {
                pkt::core::utils::throwRuntimeError("Error: " + arg + " requires an argument");
            }
            config.loggerType = argv[++i];
        }
        else if (arg == "-o" || arg == "--output")
        {
            if (i + 1 >= argc)
            {
                pkt::core::utils::throwRuntimeError("Error: " + arg + " requires an argument");
            }
            config.outputFile = argv[++i];
        }
        else if (arg == "-d" || arg == "--database")
        {
            if (i + 1 >= argc)
            {
                pkt::core::utils::throwRuntimeError("Error: " + arg + " requires an argument");
            }
            config.dbPath = argv[++i];
        }
        else
        {
            pkt::core::utils::throwRuntimeError("Error: Unknown argument: " + arg);
        }
    }

    return config;
}

// Validate and adjust configuration
void validateAndAdjustConfig(SimulationConfig& config)
{
    if (config.numHands <= 0)
    {
        pkt::core::utils::throwRuntimeError("Error: Number of hands must be positive");
    }

    if (config.startMoney <= 0)
    {
        pkt::core::utils::throwRuntimeError("Error: Start money must be positive");
    }

    // Custom strategies are required
    if (config.customStrategies.empty())
    {
        pkt::core::utils::throwRuntimeError("Error: At least one strategy must be specified with -s/--strategy");
    }

    // Calculate total players from custom strategies
    config.numPlayers = 0;
    for (const auto& spec : config.customStrategies)
    {
        config.numPlayers += spec.count;
    }

    if (config.numPlayers < 2 || config.numPlayers > 10)
    {
        pkt::core::utils::throwRuntimeError("Error: Total number of players must be between 2 and 10");
    }
}

// Create logger based on configuration
std::shared_ptr<Logger> createLogger(const std::string& loggerType)
{
    std::string lower = loggerType;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Always use console logger for now (until we're sure everything works)
    auto consoleLogger = std::make_shared<ConsoleLogger>();
    consoleLogger->setLogLevel(LogLevel::Info);
    return consoleLogger;
}

// Print simulation configuration summary
void printSimulationInfo(const SimulationConfig& config)
{
    std::cout << "Using database: " << config.dbPath << "\n";
    std::cout << "\n=== Starting Bot Statistics Simulation ===\n";
    std::cout << "Number of hands: " << config.numHands << "\n";
    std::cout << "Number of players: " << config.numPlayers << " (all bots)\n";
    std::cout << "Initial cash: " << config.startMoney << "\n";
    std::cout << "Custom strategy mix:\n";
    for (const auto& spec : config.customStrategies)
    {
        std::cout << "  - " << spec.name << ": " << spec.count << " player(s)\n";
    }
    std::cout << "\n";
}

// Start the game session based on configuration
void startGameSession(Session& session, const SimulationConfig& config)
{
    StartData startData;
    startData.numberOfPlayers = config.numPlayers;
    startData.startDealerPlayerId = StartData::AUTO_SELECT_DEALER;

    // Build BotGameData with custom strategy distribution
    BotGameData botGameData;
    botGameData.startMoney = config.startMoney;
    botGameData.firstSmallBlind = 10;

    // Build strategy distribution map
    for (const auto& spec : config.customStrategies)
    {
        botGameData.strategyDistribution[spec.name] = spec.count;
    }

    std::cout << "Using custom strategy distribution\n";
    session.startBotOnlyGameWithCustomStrategies(botGameData, startData);
}

// Run the simulation for specified number of hands
void runSimulation(Session& session, int numHands)
{
    std::cout << "Simulating hands";
    std::cout.flush();

    for (int i = 0; i < numHands; ++i)
    {
        session.startNewHand();

        // Progress indicator
        if ((i + 1) % 100 == 0)
        {
            std::cout << ".";
            std::cout.flush();
        }
        if ((i + 1) % 1000 == 0)
        {
            std::cout << " " << (i + 1) << "\n";
            std::cout.flush();
        }
    }

    std::cout << "\n\nSimulation complete!\n";
}

// Print final summary
void printFinalSummary(const SimulationConfig& config)
{
    std::cout << "\n=== Simulation Complete ===\n";
    std::cout << "Database: " << config.dbPath << "\n";
    std::cout << "Report: " << config.outputFile << "\n";
}

int main(int argc, char* argv[])
{
    try
    {
        // Parse and validate configuration
        SimulationConfig config = parseCommandLineArgs(argc, argv);
        validateAndAdjustConfig(config);

        // Create dependencies
        auto logger = createLogger(config.loggerType);
        auto handEvaluator = std::make_shared<PsimHandEvaluationEngine>();
        auto randomizer = std::make_shared<DefaultRandomizer>();

        // Create database and statistics store
        auto db = std::make_unique<SqliteDb>(config.dbPath);
        auto statsStore = std::make_shared<SqlitePlayersStatisticsStore>(std::move(db));

        // Create game events (empty for headless simulation)
        GameEvents events{};

        // Create engine factory and session
        auto factory = std::make_shared<EngineFactory>(events, *logger, *handEvaluator, *statsStore, *randomizer);
        auto session = std::make_unique<Session>(events, *factory, *logger, *handEvaluator, *statsStore, *randomizer);

        // Print configuration summary
        printSimulationInfo(config);

        // Start game session
        startGameSession(*session, config);

        // Run simulation
        runSimulation(*session, config.numHands);

        // Generate report
        generateReport(config.dbPath, config.outputFile, config.numPlayers);

        // Print final summary
        printFinalSummary(config);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
