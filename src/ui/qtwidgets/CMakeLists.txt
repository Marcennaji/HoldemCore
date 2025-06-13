# Specify the Qt installation path
set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/msvc2019_64/lib/cmake")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

# Collect all source files in the directory
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Collect all header files in the include directory
file(GLOB_RECURSE INCLUDE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

file(GLOB_RECURSE UI_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

# Generate .h and .cpp files from .ui files
qt_wrap_ui(UI_GENERATED_FILES ${UI_FILES})

# Automatically process files that require MOC
set(CMAKE_AUTOMOC ON)

# Create a static library
add_library(qtwidgets STATIC ${LIB_SOURCES} ${INCLUDE_HEADERS} ${UI_GENERATED_FILES})

# Include directories
target_include_directories(qtwidgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ui/qtwidgets
    ${CMAKE_SOURCE_DIR}/src/ui/qtwidgets/gametable
    ${CMAKE_SOURCE_DIR}/src/ui/qtwidgets/startwindow
)

# Link Qt libraries
target_link_libraries(qtwidgets PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)