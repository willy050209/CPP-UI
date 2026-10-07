# =============================================================================
# FindSkia.cmake - CMake Module to locate Google Skia 2D Graphics Library
# =============================================================================

set(SKIA_SEARCH_PATHS
    ${SKIA_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia
    $ENV{SKIA_DIR}
)

find_path(SKIA_INCLUDE_DIR
    NAMES include/core/SkCanvas.h
    PATHS ${SKIA_SEARCH_PATHS}
    PATH_SUFFIXES include
)

find_library(SKIA_LIBRARY
    NAMES skia
    PATHS ${SKIA_SEARCH_PATHS}
    PATH_SUFFIXES out/Release-x64 out/Release-arm64 out/Release lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Skia
    REQUIRED_VARS SKIA_LIBRARY SKIA_INCLUDE_DIR
)

if(Skia_FOUND)
    set(SKIA_INCLUDE_DIRS ${SKIA_INCLUDE_DIR})
    set(SKIA_LIBRARIES ${SKIA_LIBRARY})

    if(NOT TARGET Skia::Skia)
        add_library(Skia::Skia UNKNOWN IMPORTED)
        set_target_properties(Skia::Skia PROPERTIES
            IMPORTED_LOCATION "${SKIA_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SKIA_INCLUDE_DIRS}"
        )

        if(WIN32)
            set_property(TARGET Skia::Skia APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES opengl32 usp10
            )
        endif()
    endif()
endif()
