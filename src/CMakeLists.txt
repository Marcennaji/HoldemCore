# Add subdirectories
add_subdirectory(core)
add_subdirectory(infra)
add_subdirectory(ui)
add_subdirectory(third_party)

if(USE_WIDGETS)
    add_subdirectory(app)
endif()
