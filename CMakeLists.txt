cmake_minimum_required(VERSION 3.16)
project(HoldemCore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

# Only build tests for non-QML builds (widgets/console)
if(NOT USE_QML OR USE_WIDGETS)
    add_subdirectory(tests)
endif()
