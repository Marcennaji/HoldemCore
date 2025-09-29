# Add subdirectories first
add_subdirectory(actions)
add_subdirectory(cards)
add_subdirectory(hand)
add_subdirectory(probability)
add_subdirectory(game)
add_subdirectory(utils)
add_subdirectory(state)

# Automatically collect all .cpp and .h files in the current directory (excluding subdirectories)
file(GLOB ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/model/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/model/*.h
)

# Add the engine library
add_library(engine ${ENGINE_SOURCES})

# Include directories for engine
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(engine PUBLIC core actions hand cards game utils state probability)