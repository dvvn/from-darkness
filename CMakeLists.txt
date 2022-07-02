cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

# if(MSVC) idk, not defined
add_compile_options(/Zc:preprocessor /Zc:__cplusplus)
add_compile_options(/wd5103)
add_compile_options(/MP)
add_compile_definitions(NOMINMAX MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=0)
# endif()

include(FetchContent)

FetchContent_Declare(freetype GIT_REPOSITORY https://github.com/freetype/freetype.git)
set(DISABLE_FORCE_DEBUG_POSTFIX on)
set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
FetchContent_MakeAvailable(freetype)
set(FREETYPE_LIBRARY ${freetype_BINARY_DIR}/${CMAKE_BUILD_TYPE}/freetype${CMAKE_DEBUG_POSTFIX}${CMAKE_LINK_LIBRARY_SUFFIX})
get_target_property(FREETYPE_INCLUDE_DIRS freetype INTERFACE_INCLUDE_DIRECTORIES)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git)
FetchContent_Populate(imgui)
set(_IMGUI_SRC ${imgui_SOURCE_DIR})

FetchContent_Declare(xxhct GIT_REPOSITORY https://github.com/ekpyron/xxhashct.git)
FetchContent_Populate(xxhct)

FetchContent_Declare(utf8_conv GIT_REPOSITORY https://github.com/ww898/utf-cpp.git)
FetchContent_Populate(utf8_conv)

FetchContent_Declare(veque GIT_REPOSITORY https://github.com/Shmoopty/veque.git)
FetchContent_Populate(veque)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
set(JSON_MultipleHeaders on)
set(JSON_DisableEnumSerialization on)
FetchContent_MakeAvailable(json)

FetchContent_Declare(robin_hood GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git)
FetchContent_MakeAvailable(robin_hood)

#add_compile_definitions(RMLUI_NO_FILE_INTERFACE_DEFAULT)
FetchContent_Declare(rml GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 4.4)
FetchContent_MakeAvailable(rml)

# -------------------

set(_PROJ_PREFIX fd)
string(TOUPPER ${_PROJ_PREFIX} _PROJ_NAME_UPPER)
set(_PROJ_IMPL_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(_PROJ_WORK_DIR ${_PROJ_IMPL_DIR}/${_PROJ_PREFIX})

project(imgui) # -------------------

file(GLOB _IMGUI_FILES ${_IMGUI_SRC}/*.cpp)
list(APPEND _IMGUI_FILES ${_IMGUI_SRC}/misc/freetype/imgui_freetype.cpp ${_IMGUI_SRC}/backends/imgui_impl_win32.cpp ${_IMGUI_SRC}/backends/imgui_impl_dx9.cpp)

add_library(${PROJECT_NAME} STATIC ${_IMGUI_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${_IMGUI_SRC} PUBLIC ${_IMGUI_SRC}/misc/freetype PUBLIC ${_IMGUI_SRC}/backends)
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)
target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC IMGUI_ENABLE_FREETYPE
  PUBLIC IMGUI_USE_WCHAR32
  PUBLIC IMGUI_DEFINE_MATH_OPERATORS
  PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS
  PUBLIC IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
  PUBLIC ImDrawIdx=size_t)

project(${_PROJ_PREFIX}) # -------------------

file(GLOB_RECURSE _PROJ_FILES ${_PROJ_WORK_DIR}/*.ixx ${_PROJ_WORK_DIR}/*.cpp)
list(FILTER _PROJ_FILES EXCLUDE REGEX "/\_+")

add_library(${PROJECT_NAME} STATIC ${_PROJ_FILES})

set(_PROJ_KNOWN_HOOKS "")
foreach(src ${_PROJ_FILES})
  string(REGEX MATCH ".*_([0-9]+)\\.cpp" found ${src})
  if(found)
    list(APPEND _PROJ_KNOWN_HOOKS ${CMAKE_MATCH_1})
  endif()
endforeach()
message(STATUS "hooked ids - ${_PROJ_KNOWN_HOOKS}")
string(REPLACE ";" "," _PROJ_KNOWN_HOOKS_FIXED "${_PROJ_KNOWN_HOOKS}")

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${_PROJ_IMPL_DIR}
  PRIVATE ${xxhct_SOURCE_DIR}
  PRIVATE ${utf8_conv_SOURCE_DIR}/include
  PRIVATE ${veque_SOURCE_DIR}/include)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC d3d9.lib
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE robin_hood::robin_hood
  PRIVATE imgui
  PRIVATE RmlCore
  PRIVATE RmlDebugger)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE ${_PROJ_NAME_UPPER}_ROOT_DIR=${CMAKE_CURRENT_LIST_DIR}
  PRIVATE ${_PROJ_NAME_UPPER}_WORK_DIR=${_PROJ_WORK_DIR}
  PRIVATE ${_PROJ_NAME_UPPER}_KNOWN_HOOKS=${_PROJ_KNOWN_HOOKS_FIXED}
  PRIVATE RMLUI_STATIC_LIB
  PRIVATE RMLUI_DIR=${rml_SOURCE_DIR})

# -------------------

project(${_PROJ_PREFIX}_gui_test)

add_executable(${PROJECT_NAME} ${_PROJ_IMPL_DIR}/gui_test/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${_PROJ_PREFIX})
target_include_directories(${PROJECT_NAME} PRIVATE ${_PROJ_PREFIX})
