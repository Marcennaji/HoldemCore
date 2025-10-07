# Player logic

# Add subdirectory for position management
add_subdirectory(position)

# Explicitly list all source files that actually exist
set(PLAYER_SOURCES
    # Main player files
    Player.cpp
    Player.h
    PlayerListUtils.cpp
    PlayerListUtils.h
    PlayerStatistics.cpp
    PlayerStatistics.h
    PlayerStatisticsUpdater.cpp
    PlayerStatisticsUpdater.h
    PlayerFactory.cpp
    PlayerFactory.h
    CurrentHandActions.cpp
    CurrentHandActions.h
    Helpers.h
    
    # Range estimation files
    range/RangeEstimator.cpp
    range/RangeEstimator.h
    range/PreflopRangeEstimator.cpp
    range/PreflopRangeEstimator.h
    range/RangeParser.cpp
    range/RangeParser.h
    range/RangeRefiner.cpp
    range/RangeRefiner.h
    range/HandPlausibilityChecker.cpp
    range/HandPlausibilityChecker.h
    range/FlopPlausibilityChecker.cpp
    range/FlopPlausibilityChecker.h
    range/TurnPlausibilityChecker.cpp
    range/TurnPlausibilityChecker.h
    range/RiverPlausibilityChecker.cpp
    range/RiverPlausibilityChecker.h
    range/PlausibilityHelpers.cpp
    range/PlausibilityHelpers.h
    
    # Strategy files
    strategy/PlayerStrategy.h
    strategy/HumanStrategy.cpp
    strategy/HumanStrategy.h
    strategy/BotStrategyBase.cpp
    strategy/BotStrategyBase.h
    strategy/ManiacBotStrategy.cpp
    strategy/ManiacBotStrategy.h
    strategy/LooseAggressiveBotStrategy.cpp
    strategy/LooseAggressiveBotStrategy.h
    strategy/TightAggressiveBotStrategy.cpp
    strategy/TightAggressiveBotStrategy.h
    strategy/UltraTightBotStrategy.cpp
    strategy/UltraTightBotStrategy.h
    strategy/StrategyAssigner.cpp
    strategy/StrategyAssigner.h
    strategy/CurrentHandContext.h
    strategy/PreflopRangeCalculator.cpp
    strategy/PreflopRangeCalculator.h
    strategy/PokerMath.cpp
    strategy/PokerMath.h
)

# Add the player library
add_library(player ${PLAYER_SOURCES})

# Ensure threading support across toolchains (adds -pthread on MinGW, no-op on MSVC)
find_package(Threads REQUIRED)

# Include directories for player
target_include_directories(player PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src)

# Link player to core and new modules
target_link_libraries(player PUBLIC core cards probability position Threads::Threads)