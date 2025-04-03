project(engine)

# Automatically include all .cpp, .h, and .hpp files in this directory and subdirectories
file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

add_library(engine STATIC ${SOURCES} ${HEADERS})

# Include the current directory and the src directory (one level up)
target_include_directories(engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../  # Add the src directory
)