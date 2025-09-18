# Probability calculation module

set(probability_SOURCES
    DrawProbabilityCalculator.cpp
)

set(probability_HEADERS
    DrawProbabilityCalculator.h
)

add_library(probability STATIC ${probability_SOURCES} ${probability_HEADERS})

target_include_directories(probability PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
