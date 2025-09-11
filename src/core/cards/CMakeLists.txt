# Card analysis module

set(cards_SOURCES
    CardRangeAnalyzer.cpp
)

set(cards_HEADERS
    CardRangeAnalyzer.h
)

add_library(cards STATIC ${cards_SOURCES} ${cards_HEADERS})

target_include_directories(cards PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(cards
    engine # For CardUtilities
)
