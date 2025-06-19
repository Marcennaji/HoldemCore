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
    ${CMAKE_CURRENT_SOURCE_DIR}/player
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_SOURCE_DIR}/src
)
