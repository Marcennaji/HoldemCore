
# Compiler-specific warning flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wno-inconsistent-missing-override
        -Wno-microsoft-include
        -Wno-deprecated-declarations
        -Wno-delete-abstract-non-virtual-dtor
        -Wno-nonportable-include-path
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC-specific warning suppressions
    add_compile_options(/wd4267)  # Suppress size_t to int conversion warnings
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(core)
add_subdirectory(adapters/infrastructure)
add_subdirectory(ui)
add_subdirectory(third_party)

if(USE_WIDGETS OR USE_QML)
    add_subdirectory(app)
endif()
