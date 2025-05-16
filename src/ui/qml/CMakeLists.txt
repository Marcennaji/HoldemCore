find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_add_executable(pokertraining_qml
    mainQml.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    qt_add_resources(pokertraining_qml "qml_resources" FILES
        main.qml
        StartWindow.qml
        GameTable.qml
    )
endif()

target_link_libraries(pokertraining_qml
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
    core infra
)

target_compile_definitions(pokertraining_qml PRIVATE
    $<$<CONFIG:Release>:QT_NO_DEBUG>
)
