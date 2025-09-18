# Engine Game library
# Handles high-level game orchestration, board management, and pot logic

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE GAME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the game library
add_library(game ${GAME_SOURCES})

# Include directories for game
target_include_directories(game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Game library dependencies
target_link_libraries(game
    core
    actions
    hand
    cards
    utils
)