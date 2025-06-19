# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the engine library
add_library(engine ${ENGINE_SOURCES})

# Include directories for engine
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link engine to core
target_link_libraries(engine PUBLIC core infra)