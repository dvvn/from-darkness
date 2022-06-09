cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
endif()

set(_PROJECT_PREFIX fds)
string(TOUPPER ${_PROJECT_PREFIX} _PROJECT_PREFIX_UPPER)
set(_IMPL_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(_WORK_DIR ${_IMPL_DIR}/${_PROJECT_PREFIX})
set(_HOOKS_DIR ${_WORK_DIR}/hooks)

#-------------------

macro(project_message msg)
  message(STATUS "${PROJECT_NAME}: ${msg}")
endmacro()

macro(link_library vis name)
  target_link_libraries(${PROJECT_NAME} ${vis} ${name})
  target_include_directories(${PROJECT_NAME} ${vis} ${name})
  #target_compile_definitions(${PROJECT_NAME} ${vis} ${name})

  get_target_property(real_project_name ${name} ALIASED_TARGET)
  if(real_project_name STREQUAL "real_project_name-NOTFOUND")
    set(real_project_name ${name})
  endif()
  project_message("${real_project_name} installed (${vis})")
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
    project(${_PROJECT_PREFIX}_${name})
  else()
    string(REPLACE "::" "_" name_correct ${name})
    project(${_PROJECT_PREFIX}_${name_correct})
  endif()

  file(GLOB_RECURSE project_files ${dir}/*.ixx ${dir}/*.cpp)

  if(type STREQUAL "EXECUTABLE")
    add_executable(${PROJECT_NAME} ${project_files})
  else()
    if(NOT DEFINED type)
      add_library(${PROJECT_NAME} ${project_files})
    else()
      add_library(${PROJECT_NAME} ${type} ${project_files})
    endif()
    add_library(${_PROJECT_PREFIX}::${name} ALIAS ${PROJECT_NAME})
  endif()

  if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /Zc:preprocessor PUBLIC /Zc:__cplusplus)
  endif()

  get_target_property(target_type ${PROJECT_NAME} TYPE)
  project_message("created (${target_type})")
endmacro()

macro(link_partition vis name)
  link_library(${vis} ${_PROJECT_PREFIX}::${name})
endmacro()

macro(setup_hook name)
  setup_project(${_HOOKS_DIR}/targets/${name} hooks::${name})
  link_partition(PRIVATE hooks::core)
endmacro()

macro(link_hook vis name)
  link_partition(${vis} hooks::${name})
endmacro()

macro(link_external_interface vis name)
  link_partition(${vis} external_ifc::${name})
endmacro()

macro(setup_external_interface name)
  setup_project(${_WORK_DIR}/external_interfaces/${name} external_ifc::${name})
  link_partition(PRIVATE rt_modules)
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
      string(REGEX MATCH ".*_([0-9]+)\.cpp" found ${src})
      if(found)
        list(APPEND ids_list ${CMAKE_MATCH_1})
      endif()
    endforeach()
  endforeach()

  project_message("hooked ids - ${ids_list}")
  string(REPLACE ";" "," ids_str "${ids_list}")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ${_PROJECT_PREFIX_UPPER}_HOOK_IDS=${ids_str})
endmacro()

#-------------------

include(FetchContent)

FetchContent_Declare(nstd GIT_REPOSITORY https://github.com/dvvn/nstd.git)
FetchContent_MakeAvailable(nstd)

#-------------------

setup_project(${_WORK_DIR}/core)
target_include_directories(${PROJECT_NAME} PUBLIC ${_IMPL_DIR})
#link_library(PUBLIC nstd::core)
#FetchContent_Declare(xxhct GIT_REPOSITORY https://github.com/chys87/constexpr-xxh3.git GIT_TAG main)
FetchContent_Declare(xxhct GIT_REPOSITORY https://github.com/ekpyron/xxhashct.git)
FetchContent_Populate(xxhct)
target_include_directories(${PROJECT_NAME} PRIVATE ${xxhct_SOURCE_DIR}/)
#FetchContent_Declare(xxh GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git GIT_TAG dev SOURCE_SUBDIR cmake_unofficial)
#FetchContent_MakeAvailable(xxh)
#link_library(PRIVATE xxHash::xxhash)

#-------------------

setup_project(${_WORK_DIR}/text)
FetchContent_Declare(utf8_conv1 GIT_REPOSITORY https://github.com/ww898/utf-cpp.git)
FetchContent_Populate(utf8_conv1) #no cmakelist inside
target_include_directories(${PROJECT_NAME} PRIVATE ${utf8_conv1_SOURCE_DIR}/include)
link_partition(PUBLIC core)

#-------------------

setup_project(${_WORK_DIR}/logger)
link_partition(PUBLIC text)
link_library(PUBLIC nstd::format)

#-------------------

setup_project(${_WORK_DIR}/gui)
link_partition(PUBLIC logger)

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
target_link_libraries(${PROJECT_NAME} PRIVATE freetype) #unused, added to build it force

FetchContent_Declare(rml GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 4.4)
FetchContent_MakeAvailable(rml)

link_library(PRIVATE RmlCore)
link_library(PRIVATE RmlDebugger)
add_compile_definitions(${PROJECT_NAME} PRIVATE RMLUI_STATIC_LIB PRIVATE RMLUI_DIR=${rml_SOURCE_DIR})

#-------------------

setup_project(${_WORK_DIR}/mem)
link_partition(PUBLIC core)

#-------------------

setup_project(${_WORK_DIR}/runtime_modules rt_modules)
link_partition(PUBLIC mem)
link_partition(PUBLIC logger)
#link_library(PUBLIC nstd::winapi)

#-------------------

setup_project(${_HOOKS_DIR}/core hooks::core)
FetchContent_Declare(dhooks GIT_REPOSITORY https://github.com/dvvn/detour_hooks.git)
FetchContent_MakeAvailable(dhooks)
link_partition(PUBLIC logger)
link_library(PRIVATE dhooks)

#-------------------

# file(GLOB FDS_HOOKS_LIST ${_HOOKS_DIR}/targets/*/)
# foreach(dir ${FDS_HOOKS_LIST})
#   get_filename_component(hook_name ${dir} NAME)
#   setup_project(${dir} hooks::${hook_name})
#   link_library(PRIVATE ${_PROJECT_PREFIX}::hooks::core)
# endforeach()

setup_hook(winapi)
link_partition(PUBLIC gui)

#-------------------

setup_hook(directx)
link_partition(PUBLIC gui)

#-------------------

setup_external_interface(d3d9)
target_link_libraries(${PROJECT_NAME} PUBLIC d3d9.lib)

#-------------------

setup_project(${_IMPL_DIR}/gui_test gui_test EXECUTABLE)
link_hook(PRIVATE winapi)
link_hook(PRIVATE directx)
link_external_interface(PUBLIC d3d9)
write_hook_ids()

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
