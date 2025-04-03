project(third_party)

# Add include directories for poker-eval
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/poker-eval/include
    ${CMAKE_CURRENT_SOURCE_DIR}/poker-eval/include/inlines
)

# Add subdirectories for each module
add_subdirectory(tiny)
add_subdirectory(sqlite3)
add_subdirectory(poker-eval)
add_subdirectory(psim)