cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

# if(MSVC) idk, not defined
add_compile_options(/Zc:preprocessor /Zc:__cplusplus)
add_compile_options(/wd5103)
add_compile_options(/MP)
add_compile_definitions(NOMINMAX MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=0)
# endif()

set(_PROJ_PREFIX fd)
string(TOUPPER ${_PROJ_PREFIX} _PROJ_NAME_UPPER)
set(_PROJ_IMPL_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(_PROJ_WORK_DIR ${_PROJ_IMPL_DIR}/${_PROJ_PREFIX})

include(FetchContent)

project(${_PROJ_PREFIX})

file(GLOB_RECURSE _PROJ_FILES ${_PROJ_WORK_DIR}/*.ixx ${_PROJ_WORK_DIR}/*.cpp)
list(FILTER _PROJ_FILES EXCLUDE REGEX "/\\.+")

add_library(${PROJECT_NAME} STATIC ${_PROJ_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${_PROJ_IMPL_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${_PROJ_NAME_UPPER}_ROOT_DIR=${CMAKE_CURRENT_LIST_DIR} PRIVATE ${_PROJ_NAME_UPPER}_WORK_DIR=${_PROJ_WORK_DIR})

set(_PROJ_KNOWN_HOOKS "")
foreach(src ${_PROJ_FILES})
  string(REGEX MATCH ".*_([0-9]+)\\.cpp" found ${src})
  if(found)
    list(APPEND _PROJ_KNOWN_HOOKS ${CMAKE_MATCH_1})
  endif()
endforeach()
message(STATUS "hooked ids - ${_PROJ_KNOWN_HOOKS}")
string(REPLACE ";" "," _PROJ_KNOWN_HOOKS_FIXED "${_PROJ_KNOWN_HOOKS}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ${_PROJ_NAME_UPPER}_KNOWN_HOOKS=${_PROJ_KNOWN_HOOKS_FIXED})

# FetchContent_Declare(eventcpp GIT_REPOSITORY https://github.com/wqking/eventpp.git)
# FetchContent_Populate(eventcpp)
# target_include_directories(${PROJECT_NAME} PRIVATE ${eventcpp_SOURCE_DIR}/include/)
# link_library(PUBLIC nstd::core)
# FetchContent_Declare(xxhct GIT_REPOSITORY https://github.com/chys87/constexpr-xxh3.git GIT_TAG main)
FetchContent_Declare(xxhct GIT_REPOSITORY https://github.com/ekpyron/xxhashct.git)
FetchContent_Populate(xxhct)
target_include_directories(${PROJECT_NAME} PRIVATE ${xxhct_SOURCE_DIR}/)

# FetchContent_Declare(xxh GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git GIT_TAG dev SOURCE_SUBDIR cmake_unofficial)
# FetchContent_MakeAvailable(xxh)
# link_library(PRIVATE xxHash::xxhash)

FetchContent_Declare(utf8_conv GIT_REPOSITORY https://github.com/ww898/utf-cpp.git)
FetchContent_Populate(utf8_conv)
target_include_directories(${PROJECT_NAME} PRIVATE ${utf8_conv_SOURCE_DIR}/include)

# FetchContent_Declare(HarfBuzz GIT_REPOSITORY https://github.com/HarfBuzz/HarfBuzz.git GIT_TAG 4.2.1)
# FetchContent_MakeAvailable(HarfBuzz)
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
target_link_libraries(${PROJECT_NAME} PRIVATE freetype) # unused, added to build it force

FetchContent_Declare(rml GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 4.4)
FetchContent_MakeAvailable(rml)

target_link_libraries(${PROJECT_NAME} PRIVATE RmlCore PRIVATE RmlDebugger)
target_include_directories(${PROJECT_NAME} PRIVATE RmlCore)
target_compile_definitions(${PROJECT_NAME} PRIVATE RMLUI_STATIC_LIB PRIVATE RMLUI_DIR=${rml_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC d3d9.lib)

FetchContent_Declare(veque GIT_REPOSITORY https://github.com/Shmoopty/veque.git)
FetchContent_Populate(veque) #no cmakelist inside
target_include_directories(${PROJECT_NAME} PRIVATE ${veque_SOURCE_DIR}/include)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
set(JSON_MultipleHeaders on)
set(JSON_DisableEnumSerialization on)
FetchContent_MakeAvailable(json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
#target_include_directories(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

FetchContent_Declare(robin_hood GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git)
FetchContent_MakeAvailable(robin_hood)
target_include_directories(${PROJECT_NAME} PRIVATE robin_hood::robin_hood)

# -------------------

project(${_PROJ_PREFIX}_gui_test)

add_executable(${PROJECT_NAME} ${_PROJ_IMPL_DIR}/gui_test/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${_PROJ_PREFIX})
target_include_directories(${PROJECT_NAME} PRIVATE ${_PROJ_PREFIX})
