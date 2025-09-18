# Engine Hand library
# Handles hand logic, evaluation, player state management, and state transitions

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE HAND_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the hand library
add_library(hand ${HAND_SOURCES})

# Include directories for hand
target_include_directories(hand PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link to required libraries
target_link_libraries(hand PUBLIC core actions cards)