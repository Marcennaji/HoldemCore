# default values
option(USE_QML "Build QML version" OFF)
option(USE_WIDGETS "Build Widgets version" ON)

add_subdirectory(interfaces)

if(USE_QML)
    add_subdirectory(qml)
endif()

if(USE_WIDGETS)
    add_subdirectory(qtwidgets)
endif()
