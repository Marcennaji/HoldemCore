set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify the Qt installation path
set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/msvc2019_64/lib/cmake")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Collect all source files in the directory
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Collect all header files in the include directory
file(GLOB_RECURSE INCLUDE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Create a static library
add_library(qt_gui STATIC ${LIB_SOURCES} ${INCLUDE_HEADERS})

# Include directories
target_include_directories(qt_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link Qt libraries
target_link_libraries(qt_gui PRIVATE Qt6::Widgets)