# Position management module

set(position_SOURCES
    PositionManager.cpp
)

set(position_HEADERS
    PositionManager.h
)

add_library(position STATIC ${position_SOURCES} ${position_HEADERS})

target_include_directories(position PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
