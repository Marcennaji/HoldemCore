include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(qt)
add_subdirectory(third_party)

# Collect .cpp and .h files at the root of this directory
file(GLOB ROOT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the executable target
add_executable(PokerTraining ${ROOT_SOURCES})

# Add the current directory as an include directory
target_include_directories(PokerTraining PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add shared include directories
target_include_directories(PokerTraining PUBLIC
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/qt
    ${CMAKE_SOURCE_DIR}/third_party
)


find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
target_link_libraries(PokerTraining PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets engine tiny sqlite poker_eval psim qt_gui)
