find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

file(GLOB_RECURSE QML_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
message(STATUS "QML_SOURCES: ${QML_SOURCES}")

qt_add_executable(pokertraining_qml
    ${QML_SOURCES}
)
set_target_properties(pokertraining_qml PROPERTIES AUTOMOC ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    qt_add_resources(pokertraining_qml "qml_resources" FILES
        main.qml
        StartWindow.qml
        GameTable.qml
    )
endif()

target_include_directories(pokertraining_qml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(pokertraining_qml
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
    core infra core_session
)

target_compile_definitions(pokertraining_qml PRIVATE
    $<$<CONFIG:Release>:QT_NO_DEBUG>
)

set(QML_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB QML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.qml")

if(QML_FILES)
    add_custom_target(copy_qml ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QML_FILES}
        $<TARGET_FILE_DIR:pokertraining_qml>
        COMMENT "Copying QML files to output directory"
    )
    add_dependencies(pokertraining_qml copy_qml)
endif()

add_custom_command(
    TARGET pokertraining_qml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:pokertraining_qml>/data
)
