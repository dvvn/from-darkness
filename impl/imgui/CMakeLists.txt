project(ImGui CXX)

file(GLOB _IMGUI_FILES ${imgui_SOURCE_DIR}/*.cpp)
if(TARGET freetype)
    list(APPEND _IMGUI_FILES
        ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
    )
endif()

list(APPEND _IMGUI_FILES
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
)
add_library(imgui ${_IMGUI_FILES})
add_library(imgui::imgui ALIAS imgui)

target_compile_definitions(imgui PRIVATE 
    IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
    IMGUI_USE_BGRA_PACKED_COLOR #d3d9 only
)

if(TARGET freetype)
    target_link_libraries(imgui PRIVATE freetype)
    target_compile_definitions(imgui PRIVATE IMGUI_ENABLE_FREETYPE)
endif()
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_compile_definitions(imgui PUBLIC
    IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/custom_config.h"
)