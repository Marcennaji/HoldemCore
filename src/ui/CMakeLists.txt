# default values
option(USE_WIDGETS "Build Widgets version" ON)
option(USE_QML "Build QML version" ON)

if(USE_WIDGETS)
    add_subdirectory(qtwidgets)
endif()

if(USE_QML)
    add_subdirectory(qtqml)
endif()
