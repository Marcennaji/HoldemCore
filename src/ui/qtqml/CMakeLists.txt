# Specify the Qt installation path
set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/msvc2019_64/lib/cmake")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)

# Collect all source files in the directory
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Collect all header files in the include directory
file(GLOB_RECURSE INCLUDE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# QML files
file(GLOB_RECURSE QML_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.qml
)

# Qt Resource files
file(GLOB_RECURSE RESOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.qrc
)

# Configure QML module
qt_standard_project_setup(REQUIRES 6.5)

# Automatically process files that require MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Create a static library
add_library(qtqml STATIC ${LIB_SOURCES} ${INCLUDE_HEADERS} ${RESOURCE_FILES})

# Include directories
target_include_directories(qtqml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ui/qtqml
)

# Link Qt libraries and core_session library
target_link_libraries(qtqml PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml core_session infra)