
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "v1.13.0"
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(core_tests
    strategy/test_tight_aggressive.cpp
    range/RangeParserTest.cpp
    range/RangeRefinerTest.cpp
    range/CardsInRangeTest.cpp
)

target_include_directories(core_tests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/tests/strategy
    ${PROJECT_SOURCE_DIR}/tests/helpers
    ${PROJECT_SOURCE_DIR}/src
)

# Link with main engine and GoogleTest
target_link_libraries(core_tests
    PRIVATE
    player
    engine
    psim
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(core_tests)
