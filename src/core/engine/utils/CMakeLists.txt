# Engine Utils library
# Provides utility functions and helper classes for the poker engine

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE UTILS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the utils library
add_library(utils ${UTILS_SOURCES})

# Include directories for utils
target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Utils library dependencies
target_link_libraries(utils
    core
    cards
    probability
)