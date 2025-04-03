project(psim)

# Collect all source files in the lib directory
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Collect all header files in the include directory
file(GLOB_RECURSE INCLUDE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

# Create a static library for poker-eval
add_library(psim STATIC ${LIB_SOURCES} ${INCLUDE_HEADERS})

