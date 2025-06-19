# src/core/session/CMakeLists.txt

add_library(core_session STATIC
    Session.cpp
)

target_include_directories(core_session
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_session
    PUBLIC
    engine
    infra
    player
    service
    psim
    sqlite
    tiny
)
