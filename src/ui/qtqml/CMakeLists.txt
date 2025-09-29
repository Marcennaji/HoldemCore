find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml)

# Configure QML module (Qt 6 best practice)
qt_standard_project_setup(REQUIRES 6.9)

# Define QML module; lists QML and C++ sources explicitly
qt_add_qml_module(qtqml
    URI qtqml
    VERSION 1.0
    STATIC
    QML_FILES
        main.qml
    SOURCES
        QmlAppController.h
        QmlAppController.cpp
)

# Include directories
target_include_directories(qtqml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ui/qtqml
)

# Link Qt libraries and core_session library
target_link_libraries(qtqml PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml core_session infra)