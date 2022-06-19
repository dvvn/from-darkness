cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

# if(MSVC) idk, not defined
add_compile_options(/Zc:preprocessor /Zc:__cplusplus /MP)
add_compile_definitions(NOMINMAX MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=0)
# endif()

set(_PROJECT_NAME fd)
string(TOUPPER ${_PROJECT_NAME} _PROJECT_NAME_UPPER)
set(_IMPL_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(_WORK_DIR ${_IMPL_DIR}/${_PROJECT_NAME})

include(FetchContent)

project(${_PROJECT_NAME})

file(GLOB_RECURSE _PROJECT_FILES ${_WORK_DIR}/*.ixx ${_WORK_DIR}/*.cpp)
list(FILTER _PROJECT_FILES EXCLUDE REGEX "/\\.+")

add_library(${PROJECT_NAME} STATIC ${_PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${_IMPL_DIR})

set(_HOOK_IDS "")
foreach(src ${_PROJECT_FILES})
  string(REGEX MATCH ".*_([0-9]+)\\.cpp" found ${src})
  if(found)
    list(APPEND _HOOK_IDS ${CMAKE_MATCH_1})
  endif()
endforeach()
message(STATUS "hooked ids - ${_HOOK_IDS}")
string(REPLACE ";" "," _HOOK_IDS_FIXED "${_HOOK_IDS}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ${_PROJECT_NAME_UPPER}_HOOK_IDS=${_HOOK_IDS_FIXED})

# FetchContent_Declare(nstd GIT_REPOSITORY https://github.com/dvvn/nstd.git)
# FetchContent_MakeAvailable(nstd)

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

FetchContent_Declare(utf8_conv1 GIT_REPOSITORY https://github.com/ww898/utf-cpp.git)
FetchContent_Populate(utf8_conv1)
target_include_directories(${PROJECT_NAME} PRIVATE ${utf8_conv1_SOURCE_DIR}/include)

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

# -------------------

project(${_PROJECT_NAME}_gui_test)

add_executable(${PROJECT_NAME} ${_IMPL_DIR}/gui_test/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${_PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${_PROJECT_NAME})

# FetchContent_Declare(veque GIT_REPOSITORY https://github.com/Shmoopty/veque.git)
# FetchContent_Populate(veque) #no cmakelist inside
# target_include_directories(fds PRIVATE ${veque_SOURCE_DIR}/include)

# FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
# set(JSON_MultipleHeaders on)
# FetchContent_MakeAvailable(json)
# link_library(PRIVATE nlohmann_json::nlohmann_json)

# FetchContent_Declare(robin_hood GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git)
# FetchContent_MakeAvailable(robin_hood)
# link_library(PRIVATE robin_hood::robin_hood)
