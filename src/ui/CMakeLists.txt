# default values
option(USE_WIDGETS "Build Widgets version" ON)

if(USE_WIDGETS)
    add_subdirectory(qtwidgets)
endif()
