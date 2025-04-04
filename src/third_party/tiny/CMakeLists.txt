# Collect all source files in the tiny module
file(GLOB_RECURSE TINY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Create a static library for the tiny module
add_library(tiny STATIC ${TINY_SOURCES})

# Include directories for the tiny module
target_include_directories(tiny PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})