# Default values
option(USE_WIDGETS "Build Widgets version" ON)
option(USE_QML "Build QML version" ON)

if(USE_WIDGETS)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

    add_executable(HoldemCore-widgets
        main_qt_widgets.cpp
    )

    target_include_directories(HoldemCore-widgets PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )

    target_link_libraries(HoldemCore-widgets
        core_session
        infra
        qtwidgets
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
    )
endif()

if(USE_QML)
    find_package(Qt6 REQUIRED COMPONENTS Quick Qml Core Gui)

    add_executable(HoldemCore-qml
        main_qt_qml.cpp
    )

    target_include_directories(HoldemCore-qml PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )

    target_link_libraries(HoldemCore-qml
        core_session
        infra
        qtqml
        Qt6::Quick
        Qt6::Qml
        Qt6::Core
        Qt6::Gui
    )
endif()