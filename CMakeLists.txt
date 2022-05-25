cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

set(FDX_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(FDX_WORK_DIR ${FDX_DIR}/cheat)
set(FDX_CORE_DIR ${FDX_WORK_DIR}/core)
set(FDX_GUI_DIR ${FDX_WORK_DIR}/gui)
set(FDX_LOGGER_DIR ${FDX_WORK_DIR}/logger)
set(FDX_HOOKS_DIR ${FDX_WORK_DIR}/hooks)
set(FDX_HOOKS_TARGETS_DIR ${FDX_HOOKS_DIR}/targets)

macro(link_library vis name)
  target_link_libraries(${PROJECT_NAME} ${vis} ${name})
  target_include_directories(${PROJECT_NAME} ${vis} ${name})
  message(STATUS "${name} installed for ${PROJECT_NAME} as ${vis}")
endmacro()

macro(write_hook_ids)
  set(FDX_HOOK_IDS "")

  get_target_property(LIBS ${PROJECT_NAME} LINK_LIBRARIES)

  foreach(lib ${LIBS})
    if(NOT TARGET ${lib})
      continue()
    endif()

    unset(SRCS)
    get_target_property(SRCS ${lib} SOURCES)

    foreach(src ${SRCS})
      if(NOT ${src} MATCHES ".ixx$")
        continue()
      endif()
      # string(find ${src} ${FDX_HOOKS_TARGETS_DIR} starts)
      # if(NOT ${starts} EQUAL 0)
      #   continue()
      # endif()
      file(READ ${src} text) #"CHEAT_HOOK\(\w+, (\d+)\)"
      string(REGEX MATCH "CHEAT_HOOK\(.*([0-9]+)\)" found ${text})
      #message(-----${src}----${found}-----${CMAKE_MATCH_2}-----)
      list(APPEND FDX_HOOK_IDS ${CMAKE_MATCH_2})
    endforeach()
  endforeach()
  message(STATUS "ids for ${PROJECT_NAME}: ${FDX_HOOK_IDS}")
  string(REPLACE ";" "," FDX_HOOK_IDS_STR "${FDX_HOOK_IDS}")
  target_compile_definitions(${PROJECT_NAME} PRIVATE CHEAT_HOOK_IDS=${FDX_HOOK_IDS_STR})
endmacro()

macro(create_hook_library name)
  project(fdx_hooks_${name})

  unset(_FILES)
  file(GLOB _FILES ${FDX_HOOKS_TARGETS_DIR}/${name}/*)

  add_library(${PROJECT_NAME} ${_FILES})
  add_library(fdx::hooks::${name} ALIAS ${PROJECT_NAME})

  link_library(PRIVATE fdx::hooks::core)
endmacro()

#-------------------

include(FetchContent)

FetchContent_Declare(nstd GIT_REPOSITORY https://github.com/dvvn/nstd.git)
FetchContent_MakeAvailable(nstd)

#-------------------

project(fdx_core) #universal stuff

file(GLOB FDX_CORE_FILES ${FDX_CORE_DIR}/*.ixx ${FDX_CORE_DIR}/*.cpp)
add_library(${PROJECT_NAME} ${FDX_CORE_FILES})
add_library(fdx::core ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${FDX_DIR})
link_library(PUBLIC nstd::core)

project(fdx_logger) #-------

file(GLOB_RECURSE FDX_LOGGER_FILES ${FDX_LOGGER_DIR}/*.ixx ${FDX_LOGGER_DIR}/*.cpp)
add_library(${PROJECT_NAME} ${FDX_LOGGER_FILES})
add_library(fdx::logger ALIAS ${PROJECT_NAME})
link_library(PUBLIC fdx::core)
link_library(PRIVATE nstd::text)
link_library(PRIVATE nstd::format)

project(fdx_gui) #-------

file(GLOB FDX_GUI_FILES ${FDX_GUI_DIR}/*) #no headers here

add_library(${PROJECT_NAME} ${FDX_GUI_FILES})
add_library(fdx::gui ALIAS ${PROJECT_NAME})
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
add_compile_definitions(${PROJECT_NAME} PRIVATE RMLUI_STATIC_LIB)

project(fdx_hooks_core) #----------

file(GLOB FDX_HOOKS_CORE_FILES ${FDX_HOOKS_DIR}/core/*) #no headers here

add_library(${PROJECT_NAME} ${FDX_HOOKS_CORE_FILES})
add_library(fdx::hooks::core ALIAS ${PROJECT_NAME})
link_library(PUBLIC fdx::logger)

FetchContent_Declare(dhooks GIT_REPOSITORY https://github.com/dvvn/detour_hooks.git)
FetchContent_MakeAvailable(dhooks)
link_library(PRIVATE dhooks)

#----------------

create_hook_library(winapi)
link_library(PUBLIC fdx::gui)

create_hook_library(directx)
link_library(PUBLIC fdx::gui)

# project(fdx_gui_impl) #----------

# file(GLOB_RESCUE FDX_GUI_HOOKS_FILES ${FDX_HOOKS_TARGETS_DIR}/winapi/*.ixx ${FDX_CORE_DIR}/*.cpp)

# add_library(fdx_gui_test ${FDX_DIR}/gui_test/main.cpp ${FDX_HOOKS_DIR}/initializer_basic.cpp)
# link_library(PRIVATE fdx::hooks::core)
# link_library(PRIVATE fdx::gui)

macro(set_hooks_count)
  #CHEAT_HOOKS_COUNT
endmacro()

project(fdx_gui_test) #----------

add_executable(fdx_gui_test ${FDX_DIR}/gui_test/main.cpp)
link_library(PRIVATE fdx::hooks::winapi)
link_library(PRIVATE fdx::hooks::directx)
target_link_libraries(${PROJECT_NAME} PRIVATE d3d9.lib)
write_hook_ids()

# project(fdx)

# file(GLOB_RECURSE FDX_FILES ${FDX_WORK_DIR}/*.cpp ${FDX_WORK_DIR}/*.ixx ${FDX_WORK_DIR}/*.h)

# add_library(fdx ${FDX_FILES})
# target_include_directories(fdx PUBLIC ${FDX_DIR})

# FetchContent_Declare(veque GIT_REPOSITORY https://github.com/Shmoopty/veque.git)
# FetchContent_Populate(veque) #no cmakelist inside
# target_include_directories(fdx PRIVATE ${veque_SOURCE_DIR}/include)

# FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git)
# set(JSON_MultipleHeaders on)
# FetchContent_MakeAvailable(json)
# link_library(PRIVATE nlohmann_json::nlohmann_json)

# FetchContent_Declare(robin_hood GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git)
# FetchContent_MakeAvailable(robin_hood)
# link_library(PRIVATE robin_hood::robin_hood)
