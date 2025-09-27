# Infrastructure logic (e.g., logging)

# Automatically collect all .cpp and .h files in the current directory
file(GLOB_RECURSE INFRA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the infra library
add_library(infra ${INFRA_SOURCES})

# Include directories for infra
target_include_directories(infra 
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            PRIVATE ${CMAKE_SOURCE_DIR}/src
            )

# Link infra to third party libs
target_link_libraries(infra PRIVATE psim sqlite)

# Link infra to engine so it can use engine interfaces
target_link_libraries(infra PUBLIC engine)