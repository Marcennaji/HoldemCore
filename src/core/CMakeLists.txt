# Core game logic

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(player)
add_subdirectory(session)
add_subdirectory(services)

# Create a library for core
add_library(core INTERFACE)

# Include directories for core
target_include_directories(core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/actions
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/cards
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/hand
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/game
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/state
    ${CMAKE_CURRENT_SOURCE_DIR}/player
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_SOURCE_DIR}/src
)
