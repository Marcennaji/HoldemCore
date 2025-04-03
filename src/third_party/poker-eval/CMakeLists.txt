project(poker_eval)

# Collect all source files in the lib directory
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.c
)

# Collect all header files in the include directory
file(GLOB_RECURSE INCLUDE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

# Create a static library for poker-eval
add_library(poker_eval STATIC ${LIB_SOURCES} ${INCLUDE_HEADERS})

# Include the include directory for this library
target_include_directories(poker_eval PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)