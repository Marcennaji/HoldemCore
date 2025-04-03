project(sqlite)

# Collect all source files in the sqlite module
file(GLOB_RECURSE sqlite_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Create a static library for the sqlite module
add_library(sqlite STATIC ${sqlite_SOURCES})

# Include directories for the sqlite module
target_include_directories(sqlite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})