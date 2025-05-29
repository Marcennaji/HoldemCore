# Player logic

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE PLAYER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the player library
add_library(player ${PLAYER_SOURCES})

# Include directories for player
target_include_directories(player PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src)

# Link player to core
target_link_libraries(player PUBLIC core)