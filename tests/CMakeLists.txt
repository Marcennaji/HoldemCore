
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "v1.13.0"
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# --- Collect test sources ---
# Allow tests to live in nested subfolders (e.g., unit/engine/...).
file(GLOB_RECURSE UNIT_TEST_SOURCES
    "unit/*.cpp"
    "unit/**/*.cpp"
)
file(GLOB_RECURSE UNIT_TEST_HEADERS
    "unit/*.h"
    "unit/**/*.h"
)

file(GLOB_RECURSE E2E_TEST_SOURCES
    "e2e/*.cpp"
    "e2e/**/*.cpp"
)
file(GLOB_RECURSE E2E_TEST_HEADERS
    "e2e/*.h"
    "e2e/**/*.h"
)

file(GLOB_RECURSE COMMON_SOURCES "common/*.cpp")
file(GLOB_RECURSE COMMON_HEADERS "common/*.h")

# --- Unit tests executable ---
add_executable(unit_tests
    ${UNIT_TEST_SOURCES}
    ${UNIT_TEST_HEADERS}
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)

target_include_directories(unit_tests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/tests
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(unit_tests
    PRIVATE
    player
    engine
    service
    infra
    core_session
    gtest_main
    gmock_main
)

# --- E2E tests executable ---
add_executable(e2e_tests
    ${E2E_TEST_SOURCES}
    ${E2E_TEST_HEADERS}
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)

target_include_directories(e2e_tests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/tests
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(e2e_tests
    PRIVATE
    player
    engine
    service
    infra
    core_session
    gtest_main
)

# --- GoogleTest integration ---
include(GoogleTest)
gtest_discover_tests(unit_tests)
gtest_discover_tests(e2e_tests)
