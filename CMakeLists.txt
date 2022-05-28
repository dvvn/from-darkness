cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

set(FDX_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(FDX_WORK_DIR ${FDX_DIR}/cheat)
set(FDX_HOOKS_DIR ${FDX_WORK_DIR}/hooks)

#-------------------

macro(link_library vis name)
  target_link_libraries(${PROJECT_NAME} ${vis} ${name})
  target_include_directories(${PROJECT_NAME} ${vis} ${name})
  message(STATUS "${PROJECT_NAME}: ${name} installed (${vis})")
endmacro()

macro(setup_project dir)
  set(extra_args ${ARGN})
  list(LENGTH extra_args extra_count)

  unset(name)
  unset(type)

  if(extra_count GREATER 0)
    list(GET extra_args 0 name)
  endif()
  if(extra_count GREATER 1)
    list(GET extra_args 1 type)
  endif()

  if(NOT DEFINED name)
    get_filename_component(name ${dir} NAME)
    project(fdx_${name})
  else()
    string(REPLACE "::" "_" name_correct ${name})
    project(fdx_${name_correct})
  endif()

  file(GLOB_RECURSE _FILES ${dir}/*.ixx ${dir}/*.cpp)

  if(type STREQUAL "EXECUTABLE")
    add_executable(${PROJECT_NAME} ${_FILES})
  else()
    if(NOT DEFINED type)
      add_library(${PROJECT_NAME} ${_FILES})
    else()
      add_library(${PROJECT_NAME} ${type} ${_FILES})
    endif()
    add_library(fdx::${name} ALIAS ${PROJECT_NAME})
  endif()

  get_target_property(target_type ${PROJECT_NAME} TYPE)
  message(STATUS "${PROJECT_NAME}: created (${target_type})")
endmacro()

macro(setup_hook name)
  setup_project(${FDX_HOOKS_DIR}/targets/${name} hooks::${name})
  link_library(PRIVATE fdx::hooks::core)
endmacro()

macro(link_hook vis name)
  link_library(${vis} fdx::hooks::${name})
endmacro()

macro(link_external_interface vis name)
  link_library(${vis} fdx::external_ifc::${name})
endmacro()

macro(setup_external_interface name)
  setup_project(${FDX_WORK_DIR}/external_interfaces/${name} external_ifc::${name})
  link_library(PRIVATE fdx::rt_modules)
endmacro()

macro(write_hook_ids)
  set(ids_list "")

  get_target_property(libs_list ${PROJECT_NAME} LINK_LIBRARIES)

  foreach(lib ${libs_list})
    if(NOT TARGET ${lib})
      continue()
    endif()

    unset(sources_list)
    get_target_property(sources_list ${lib} SOURCES)

    foreach(src ${sources_list})
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
      list(APPEND ids_list ${CMAKE_MATCH_2})
    endforeach()
  endforeach()

  message(STATUS "ids for ${PROJECT_NAME}: ${ids_list}")
  string(REPLACE ";" "," ids_str "${ids_list}")
  target_compile_definitions(${PROJECT_NAME} PRIVATE CHEAT_HOOK_IDS=${ids_str})
endmacro()

#-------------------

include(FetchContent)

FetchContent_Declare(nstd GIT_REPOSITORY https://github.com/dvvn/nstd.git)
FetchContent_MakeAvailable(nstd)

#-------------------

setup_project(${FDX_WORK_DIR}/core)
target_include_directories(${PROJECT_NAME} PUBLIC ${FDX_DIR})
link_library(PUBLIC nstd::core)

#-------------------

setup_project(${FDX_WORK_DIR}/logger)
link_library(PUBLIC fdx::core)
link_library(PUBLIC nstd::text)
link_library(PUBLIC nstd::format)

#-------------------

setup_project(${FDX_WORK_DIR}/gui)
link_library(PUBLIC fdx::logger)

#FetchContent_Declare(HarfBuzz GIT_REPOSITORY https://github.com/HarfBuzz/HarfBuzz.git GIT_TAG 4.2.1)
#FetchContent_MakeAvailable(HarfBuzz)

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
link_library(INTERFACE freetype) #unused, added to build it force

FetchContent_Declare(rml GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 4.4)
FetchContent_MakeAvailable(rml)

link_library(PRIVATE RmlCore)
link_library(PRIVATE RmlDebugger)
add_compile_definitions(${PROJECT_NAME} PRIVATE RMLUI_STATIC_LIB PRIVATE RMLUI_DIR=${rml_SOURCE_DIR})

#-------------------

setup_project(${FDX_WORK_DIR}/runtime_modules rt_modules)
link_library(PUBLIC fdx::logger)
link_library(PUBLIC nstd::winapi)

#-------------------

setup_project(${FDX_HOOKS_DIR}/core hooks::core)
FetchContent_Declare(dhooks GIT_REPOSITORY https://github.com/dvvn/detour_hooks.git)
FetchContent_MakeAvailable(dhooks)
link_library(PUBLIC fdx::logger)
link_library(PRIVATE dhooks)

#-------------------

# file(GLOB FDX_HOOKS_LIST ${FDX_HOOKS_DIR}/targets/*/)
# foreach(dir ${FDX_HOOKS_LIST})
#   get_filename_component(hook_name ${dir} NAME)
#   setup_project(${dir} hooks::${hook_name})
#   link_library(PRIVATE fdx::hooks::core)
# endforeach()

setup_hook(winapi)
link_library(PUBLIC fdx::gui)

#-------------------

setup_hook(directx)
link_library(PUBLIC fdx::gui)

#-------------------

setup_external_interface(d3d9)
target_link_libraries(${PROJECT_NAME} PUBLIC d3d9.lib)

#-------------------

setup_project(${FDX_DIR}/gui_test gui_test EXECUTABLE)
link_hook(PRIVATE winapi)
link_hook(PRIVATE directx)
link_external_interface(PUBLIC d3d9)
write_hook_ids()

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
