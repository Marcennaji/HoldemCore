# Engine State library
# Manages finite state machine for betting rounds (preflop, flop, turn, river, post-river)

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE STATE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the state library
add_library(state ${STATE_SOURCES})

# Include directories for state
target_include_directories(state PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# State library dependencies
target_link_libraries(state
    core
    actions
    hand
)