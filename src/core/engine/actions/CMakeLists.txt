# Engine Actions library
# Handles player action validation, invalid action handling, and betting actions

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE ACTIONS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the actions library
add_library(actions ${ACTIONS_SOURCES})

# Include directories for actions
target_include_directories(actions PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link to required libraries
target_link_libraries(actions PUBLIC core)