find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

add_executable(QtHoldemCore
    main_desktop_app_qt.cpp
)

target_include_directories(QtHoldemCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(QtHoldemCore
    core_session
    infra
    qtwidgets
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)