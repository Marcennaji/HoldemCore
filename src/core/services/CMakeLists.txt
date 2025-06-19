file(GLOB_RECURSE SERVICES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Add the service library
add_library(service ${SERVICES_SOURCES})

# Include directories for service
target_include_directories(service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link service to core
target_link_libraries(service PUBLIC core)