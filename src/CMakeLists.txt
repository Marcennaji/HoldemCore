
add_compile_options(
    -Wno-inconsistent-missing-override
    -Wno-microsoft-include
    -Wno-deprecated-declarations
    -Wno-delete-abstract-non-virtual-dtor
    -Wno-nonportable-include-path
)
add_definitions(-DLOG_POKER_EXEC)

# add_definitions(-DLOG_POKER_CONSOLE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(core)
add_subdirectory(infra)
add_subdirectory(ui)
add_subdirectory(third_party)

if(USE_WIDGETS)
    add_subdirectory(app)
endif()
