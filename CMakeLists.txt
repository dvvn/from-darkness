cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

include(FetchContent)

FetchContent_Declare(nstd GIT_REPOSITORY https://github.com/dvvn/nstd.git)
FetchContent_MakeAvailable(nstd)

macro(link_library vis name)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${vis} ${name})
  target_include_directories(${CMAKE_PROJECT_NAME} ${vis} ${name})
  message("${name} installed for ${CMAKE_PROJECT_NAME} as ${vis}")
endmacro()

set(FDX_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(FDX_WORK_DIR ${FDX_DIR}/cheat)
set(FDX_CORE_DIR ${FDX_WORK_DIR}/core)
set(FDX_GUI_DIR ${FDX_WORK_DIR}/gui)
set(FDX_LOGGER_DIR ${FDX_WORK_DIR}/logger)
set(FDX_HOOKS_DIR ${FDX_WORK_DIR}/hooks)

project(fdx_core) #universal stuff

file(GLOB FDX_CORE_FILES ${FDX_CORE_DIR}/*.ixx ${FDX_CORE_DIR}/*.cpp)
add_library(fdx_core ${FDX_CORE_FILES})
add_library(fdx::core ALIAS fdx_core)
target_include_directories(fdx_core PUBLIC ${FDX_DIR})
link_library(PUBLIC nstd::core)

project(fdx_logger) #-------

file(GLOB FDX_LOGGER_FILES ${FDX_LOGGER_DIR}/*.ixx ${FDX_LOGGER_DIR}/*.cpp)
add_library(fdx_logger ${FDX_LOGGER_FILES})
add_library(fdx::logger ALIAS fdx_logger)
link_library(PUBLIC fdx::core)
link_library(PRIVATE nstd::text)
link_library(PRIVATE nstd::format)

project(fdx_gui) #-------

file(GLOB FDX_GUI_FILES ${FDX_GUI_DIR}/*) #no headers here

add_library(fdx_gui ${FDX_GUI_FILES})
add_library(fdx::gui ALIAS fdx_gui)
link_library(PUBLIC fdx::logger)

FetchContent_Declare(hazzbuzz GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git GIT_TAG 4.2.1)
FetchContent_MakeAvailable(hazzbuzz)

FetchContent_Declare(freetype GIT_REPOSITORY https://github.com/freetype/freetype.git)
set(DISABLE_FORCE_DEBUG_POSTFIX on)
FetchContent_MakeAvailable(freetype)
set(FREETYPE_LIBRARY ${freetype_BINARY_DIR}/${CMAKE_BUILD_TYPE}/freetype${CMAKE_DEBUG_POSTFIX}${CMAKE_LINK_LIBRARY_SUFFIX})
get_target_property(FREETYPE_INCLUDE_DIRS freetype INTERFACE_INCLUDE_DIRECTORIES)

FetchContent_Declare(rml GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 4.4)
FetchContent_MakeAvailable(rml)
link_library(PRIVATE RmlCore)
link_library(PRIVATE RmlDebugger)
add_compile_definitions(fdx_gui PRIVATE RMLUI_STATIC_LIB)

project(fdx_hooks_core) #----------

file(GLOB FDX_HOOKS_CORE_FILES ${FDX_HOOKS_DIR}/core/*) #no headers here

add_library(fdx_hooks_core ${FDX_HOOKS_CORE_FILES})
add_library(fdx::hooks::core ALIAS fdx_hooks_core)
link_library(PRIVATE fdx::logger)

FetchContent_Declare(dhooks GIT_REPOSITORY https://github.com/dvvn/detour_hooks.git)
FetchContent_MakeAvailable(dhooks)
link_library(PRIVATE dhooks)

project(fdx)

file(GLOB_RECURSE FDX_FILES ${FDX_WORK_DIR}/*.cpp ${FDX_WORK_DIR}/*.ixx ${FDX_WORK_DIR}/*.h)

add_library(fdx ${FDX_FILES})
target_include_directories(fdx PUBLIC ${FDX_DIR})

FetchContent_Declare(veque GIT_REPOSITORY https://github.com/Shmoopty/veque.git)
FetchContent_Populate(veque) #no cmakelist inside
target_include_directories(fdx PRIVATE ${veque_SOURCE_DIR}/include)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
set(JSON_MultipleHeaders on)
FetchContent_MakeAvailable(json)
link_library(PRIVATE nlohmann_json::nlohmann_json)

FetchContent_Declare(robin_hood GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git)
FetchContent_MakeAvailable(robin_hood)
link_library(PRIVATE robin_hood::robin_hood)
