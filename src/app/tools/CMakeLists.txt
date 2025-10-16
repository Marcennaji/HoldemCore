# Bot Statistics Generator Tool
add_executable(bot_statistics_generator
    bot_statistics_generator.cpp
)

target_link_libraries(bot_statistics_generator
    PRIVATE
    core_session
    infrastructure
)

target_include_directories(bot_statistics_generator
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Set C++20 standard
set_target_properties(bot_statistics_generator PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
