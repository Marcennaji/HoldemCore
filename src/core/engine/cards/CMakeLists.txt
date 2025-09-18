# Card analysis module

file(GLOB_RECURSE CARDS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(cards STATIC ${CARDS_SOURCES})

target_include_directories(cards PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(cards
    engine # For CardUtilities
)
