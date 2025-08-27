
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "v1.13.0"
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Add all test cpp files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_executable(core_tests
    ${TEST_SOURCES}
)

target_include_directories(core_tests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/tests
    ${PROJECT_SOURCE_DIR}/tests/strategy
    ${PROJECT_SOURCE_DIR}/tests/helpers
    ${PROJECT_SOURCE_DIR}/tests/engine
    ${PROJECT_SOURCE_DIR}/tests/range
    ${PROJECT_SOURCE_DIR}/tests/cards-value
    ${PROJECT_SOURCE_DIR}/src
)

# Link with engine and gtest
target_link_libraries(core_tests
    PRIVATE
    player
    engine
    service
    infra
    gtest_main
)

# Disable GoogleTest SEH handling for this target
target_compile_definitions(core_tests
    PRIVATE
    GTEST_HAS_SEH=0
)

include(GoogleTest)
gtest_discover_tests(core_tests)
