# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
message(STATUS "ENGINE_SOURCES (to be included in engine library):")

foreach(file ${ENGINE_SOURCES})
    message(STATUS "  ${file}")
endforeach()

# Add the engine library
add_library(engine ${ENGINE_SOURCES})

# Include directories for engine
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(engine PUBLIC core player)