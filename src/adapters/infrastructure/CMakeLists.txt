# Infrastructure adapters implementation

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE INFRASTRUCTURE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the infrastructure library
add_library(infrastructure ${INFRASTRUCTURE_SOURCES})

# Ensure threading support for platforms that require it (e.g., MinGW)
find_package(Threads REQUIRED)

# Include directories for infrastructure
target_include_directories(infrastructure 
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            PRIVATE ${CMAKE_SOURCE_DIR}/src
            )

# Link infrastructure to third party libs
target_link_libraries(infrastructure PRIVATE psim sqlite Threads::Threads)

# Link infrastructure to engine so it can use engine ports
target_link_libraries(infrastructure PUBLIC engine)