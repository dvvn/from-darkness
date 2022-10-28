cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off) 
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) 

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
  set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_C_STANDARD 17) 
set(CMAKE_C_STANDARD_REQUIRED on)

set(_PROJ_NAME fd)
string(TOUPPER ${_PROJ_NAME} _PROJ_NAME_UPPER)
set(_PROJ_IMPL_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(_PROJ_WORK_DIR ${_PROJ_IMPL_DIR}/${_PROJ_NAME})

project(${_PROJ_NAME})

include(ProcessorCount)
ProcessorCount(_CPUS)
message(DEBUG "cpu count: ${_CPUS}")

if(MSVC)
  add_compile_options(/Zc:preprocessor /Zc:__cplusplus)
  add_compile_options(/wd5103 /wd4291 /wd4250)
  add_compile_options(/MP${_CPUS} /EHsc)
  add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=0)
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET off)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/.deps)

FetchContent_Declare(freetype GIT_REPOSITORY https://github.com/freetype/freetype.git)
set(DISABLE_FORCE_DEBUG_POSTFIX on)
set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
FetchContent_MakeAvailable(freetype)

#[[ set(FREETYPE_LIBRARY ${freetype_BINARY_DIR}/${CMAKE_BUILD_TYPE}/freetype${CMAKE_DEBUG_POSTFIX}.lib)
get_target_property(FREETYPE_INCLUDE_DIRS freetype INTERFACE_INCLUDE_DIRECTORIES)
FetchContent_Declare(rml GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 4.4)
FetchContent_MakeAvailable(rml)
unset(FREETYPE_LIBRARY)
unset(FREETYPE_INCLUDE_DIRS) ]]

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git)
FetchContent_Populate(imgui)

FetchContent_Declare(xxhct GIT_REPOSITORY https://github.com/ekpyron/xxhashct.git)
FetchContent_Populate(xxhct)

FetchContent_Declare(utf8_conv GIT_REPOSITORY https://github.com/ww898/utf-cpp.git)
FetchContent_Populate(utf8_conv)

FetchContent_Declare(veque GIT_REPOSITORY #[[ https://github.com/Shmoopty/veque.git ]] https://github.com/dvvn/veque.git)
FetchContent_Populate(veque)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
set(JSON_MultipleHeaders on)
set(JSON_DisableEnumSerialization on)
FetchContent_MakeAvailable(json)

FetchContent_Declare(robin_hood GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git)
FetchContent_MakeAvailable(robin_hood)

FetchContent_Declare(function2 GIT_REPOSITORY #[[ https://github.com/Naios/function2.git ]] https://github.com/dvvn/function2.git)
FetchContent_MakeAvailable(function2)

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git)
set(FMT_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(subhook GIT_REPOSITORY https://github.com/Zeex/subhook.git)
set(SUBHOOK_STATIC on)
set(SUBHOOK_INSTALL off)
set(SUBHOOK_TESTS off)
FetchContent_MakeAvailable(subhook)

# -------------------

set(_DEFAULT_FILES_FILTER "/\_+")

file(GLOB _IMGUI_FILES ${imgui_SOURCE_DIR}/*.cpp)
list(APPEND _IMGUI_FILES ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp)
if(MSVC)
  list(APPEND _IMGUI_FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp)
else()

endif()
add_library(imgui STATIC ${_IMGUI_FILES})
target_link_libraries(imgui PRIVATE freetype)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="${_PROJ_WORK_DIR}/gui/config.h")

#-------

file(GLOB_RECURSE _PROJ_FILES ${_PROJ_WORK_DIR}/*.cpp ${_PROJ_WORK_DIR}/*.ixx)
list(FILTER _PROJ_FILES EXCLUDE REGEX ${_DEFAULT_FILES_FILTER})
add_library(impl STATIC ${_PROJ_FILES})
target_link_libraries(impl PUBLIC function2::function2 robin_hood::robin_hood nlohmann_json::nlohmann_json subhook::subhook fmt::fmt imgui)
target_include_directories(impl PUBLIC ${_PROJ_IMPL_DIR} PRIVATE ${xxhct_SOURCE_DIR} ${utf8_conv_SOURCE_DIR}/include ${veque_SOURCE_DIR}/include)
target_compile_definitions(impl PUBLIC ${_PROJ_NAME_UPPER}_WORK_DIR=${_PROJ_WORK_DIR} ${_PROJ_NAME_UPPER}_ROOT_DIR=${_PROJ_IMPL_DIR} #[[ RMLUI_STATIC_LIB RMLUI_DIR=${rml_SOURCE_DIR} ]])

file(GLOB_RECURSE _GUI_TEST_FILES ${_PROJ_IMPL_DIR}/gui_test/*.cpp ${_PROJ_IMPL_DIR}/gui_test/*.ixx)
add_executable(gui_test ${_GUI_TEST_FILES})
target_link_libraries(gui_test impl d3d9.lib)
target_compile_definitions(gui_test PRIVATE ${_PROJ_NAME_UPPER}_GUI_TEST)

file(GLOB_RECURSE _LIBRARY_FILES ${_PROJ_IMPL_DIR}/library/*.cpp ${_PROJ_IMPL_DIR}/library/*.ixx)
add_library(library SHARED ${_LIBRARY_FILES})
target_link_libraries(library impl)
target_compile_definitions(library PRIVATE ${_PROJ_NAME_UPPER}_LIBRARY)
