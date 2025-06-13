# default values
option(USE_WIDGETS "Build Widgets version" ON)

add_subdirectory(interfaces)

if(USE_WIDGETS)
    add_subdirectory(qtwidgets)
endif()
