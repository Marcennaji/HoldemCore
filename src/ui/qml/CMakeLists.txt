find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_add_executable(pokertraining_qml
    main.cpp
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

set(QML_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB QML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.qml")
add_custom_target(copy_qml ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${QML_FILES}
    $<TARGET_FILE_DIR:pokertraining_qml>
    BYPRODUCTS ${QML_FILES}
    COMMENT "Copying QML files to output directory"
)

add_dependencies(pokertraining_qml copy_qml)

add_custom_command(
    TARGET pokertraining_qml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:pokertraining_qml>/data
)
