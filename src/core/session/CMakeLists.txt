# src/core/session/CMakeLists.txt

file(GLOB_RECURSE SESSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(core_session STATIC
    ${SESSION_SOURCES}
)

target_include_directories(core_session
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_session
    PUBLIC
    engine
    player
    service
)
