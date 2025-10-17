# Default values
option(USE_WIDGETS "Build Widgets version" ON)
option(USE_QML "Build QML version" ON)

if(USE_WIDGETS)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

    add_executable(HoldemCore-widgets
        qtwidgets/main.cpp
        qtwidgets/AppFactory.cpp
        qtwidgets/AppFactory.h
        DependenciesFactory.cpp
        DependenciesFactory.h
    )

    target_include_directories(HoldemCore-widgets PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )

    target_link_libraries(HoldemCore-widgets
        core_session
        infrastructure
        qtwidgets
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
    )
endif()

if(USE_QML)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Qml Quick QuickControls2)

    add_executable(HoldemCore-qml
        qtqml/main.cpp
        qtqml/AppFactory.cpp
        qtqml/AppFactory.h
        DependenciesFactory.cpp
        DependenciesFactory.h
    )

    target_include_directories(HoldemCore-qml PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )

    target_link_libraries(HoldemCore-qml
        core_session
        infrastructure
        qtqml
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
    )

    # Copy card images to build directory for runtime access
    set(CARDS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/ui/shared/resources/images/cards")
    set(CARDS_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/cards")

    add_custom_command(TARGET HoldemCore-qml POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CARDS_SOURCE_DIR}
        ${CARDS_DEST_DIR}
        COMMENT "Copying card images to build directory"
    )
endif()

# Add tools subdirectory
add_subdirectory(tools)
