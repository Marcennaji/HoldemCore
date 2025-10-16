# HoldemCore Qt Quick/QML UI CMakeLists
# Requires Qt6 Quick, QML, and related components

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick Sql)

# Collect C++ source files (controllers, bridges, view models)
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/controller/*.cpp
)

# Collect C++ header files
file(GLOB_RECURSE INCLUDE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/controller/*.h
)

# Enable automatic MOC, RCC, and UIC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 QML module setup
qt_add_library(qtqml STATIC
    ${LIB_SOURCES}
    ${INCLUDE_HEADERS}
)

# Add QML module with resources
# Note: QML_FILES must be specified with relative paths from CMAKE_CURRENT_SOURCE_DIR
qt_add_qml_module(qtqml
    URI HoldemCore
    VERSION 1.0
    QML_FILES
    qml/Main.qml
    qml/SetupScreen.qml
    qml/TableScreen.qml
    qml/components/PokerTable.qml
    qml/components/PlayerWidget.qml
    qml/components/CardWidget.qml
    qml/components/ActionPanel.qml
    RESOURCE_PREFIX "/"
)

# Include directories
target_include_directories(qtqml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ui/qtqml
    ${CMAKE_SOURCE_DIR}/src/ui/qtqml/controller
)

# Link Qt libraries and core libraries
target_link_libraries(qtqml
    PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::Sql
    PRIVATE
    core_session
)
