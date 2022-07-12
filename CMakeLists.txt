cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
  set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
  #set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

include(ProcessorCount)
ProcessorCount(_CPUS)
message("cpu count: ${_CPUS}")

# if(MSVC) idk, not defined
add_compile_options(/Zc:preprocessor /Zc:__cplusplus)
add_compile_options(/wd5103)
add_compile_options(/MP${_CPUS})
add_compile_options(/permissive-)
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=0)
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

set(FREETYPE_LIBRARY ${freetype_BINARY_DIR}/${CMAKE_BUILD_TYPE}/freetype${CMAKE_DEBUG_POSTFIX}.lib)
get_target_property(FREETYPE_INCLUDE_DIRS freetype INTERFACE_INCLUDE_DIRECTORIES)
FetchContent_Declare(rml GIT_REPOSITORY https://github.com/mikke89/RmlUi.git GIT_TAG 4.4)
FetchContent_MakeAvailable(rml)
unset(FREETYPE_LIBRARY)
unset(FREETYPE_INCLUDE_DIRS)

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

# -------------------

set(_PROJ_NAME fd)
string(TOUPPER ${_PROJ_NAME} _PROJ_NAME_UPPER)
set(_PROJ_IMPL_DIR ${CMAKE_CURRENT_LIST_DIR}/impl)
set(_PROJ_WORK_DIR ${_PROJ_IMPL_DIR}/${_PROJ_NAME})

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

# -------------------

function(get_files _path)
  set(_list "")

  foreach(_item ${_path})
    if(IS_DIRECTORY ${_item})
      file(GLOB_RECURSE _files_tmp ${_item}/*.ixx ${_item}/*.cpp)
      list(APPEND _list ${_files_tmp})
    else()
      list(APPEND _list ${_item})
    endif()
  endforeach()
  list(FILTER _list EXCLUDE REGEX "/\_+")
  #message(DEBUG "Files found in ${_path}: ${_list}")
  set(_FILES ${_list} PARENT_SCOPE)
endfunction()

function(target_include_directory _target _type _dir)
  target_include_directories(${_target} ${_type} ${_dir})
  message(DEBUG "${_target}: directory ${_dir} included (${_type})")
endfunction()

function(target_link_library _target _type _lib)
  target_link_libraries(${_target} ${_type} ${_lib})
  message(DEBUG "${_target}: library ${_lib} linked (${_type})")
endfunction()

function(target_compile_definition _target _type _def)
  target_compile_definitions(${_target} ${_type} ${_def})
  message(DEBUG "${_target}: macro ${_def} added (${_type})")
endfunction()

function(project_part)
  cmake_parse_arguments(PART "" "NAME;TYPE" "FILES;INCLUDE;LINK;DEFINE" ${ARGN})

  if(NOT PART_NAME)
    message(FATAL_ERROR "Name not set!")
  else()
    set(_name ${PART_NAME})
  endif()

  #---------------

  if(NOT PART_FILES)
    set(PART_FILES ${_PROJ_WORK_DIR}/${_name})
  endif()

  get_files(${PART_FILES})
  list(LENGTH _FILES _files_count)
  if(_files_count EQUAL 0)
    if(PART_TYPE AND NOT PART_TYPE STREQUAL "INTERFACE")
      message(FATAL_ERROR "Incorrect TYPE!")
    else()
      set(_type INTERFACE)
    endif()
  else()
    if(PART_TYPE)
      set(_type ${PART_TYPE})
    else()
      set(_type STATIC)
    endif()
  endif()

  set(_target ${_PROJ_NAME}_${_name})
  if(_type STREQUAL "EXECUTABLE")
    add_executable(${_target} ${_FILES})
    set(_link_type_root PRIVATE)
  else()
    add_library(${_target} ${_type} ${_FILES})
    if(_type STREQUAL "INTERFACE")
      set(_link_type_root ${_type})
    else()
      set(_link_type_root PUBLIC)
    endif()
  endif()

  #---------------

  if(PART_INCLUDE)
    set(_include_type ${_link_type_root})

    foreach(_item ${PART_INCLUDE})
      if(EXISTS ${_item})
        target_include_directory(${_target} ${_include_type} ${_item})
      elseif(_item MATCHES "PRIVATE|PUBLIC|INTERFACE")
        set(_include_type ${_item})
      else()
        set(_own_dir ${_PROJ_WORK_DIR}/${_item})
        if(NOT EXISTS ${_own_dir})
          message(FATAL_ERROR "Directory ${_item} not found!")
        endif()
        target_include_directory(${_target} ${_include_type} ${_own_dir})
      endif()
    endforeach()
  endif()

  #---------------

  if(PART_LINK)
    set(_link_type ${_link_type_root})

    foreach(_lib ${PART_LINK})
      if(_lib MATCHES "PRIVATE|PUBLIC|INTERFACE")
        set(_link_type ${_lib})
      else()
        set(_self_target ${_PROJ_NAME}_${_lib})
        set(_external_target ${_lib}::${_lib})
        if(TARGET ${_self_target})
          target_link_library(${_target} ${_link_type} ${_self_target})
        elseif(TARGET ${_external_target})
          target_link_library(${_target} ${_link_type} ${_external_target})
        elseif(TARGET ${_lib} OR ${_lib} MATCHES ".*\.+")
          target_link_library(${_target} ${_link_type} ${_lib})
        else()
          message(FATAL_ERROR "Library ${_lib} not found!")
        endif()
      endif()
    endforeach()
  endif()

  #---------------

  if(PART_DEFINE)
    set(_def_type ${_link_type_root})

    foreach(_def ${PART_DEFINE})
      if(_def MATCHES "PRIVATE|PUBLIC|INTERFACE")
        set(_def_type ${_def})
      else()
        target_compile_definition(${_target} ${_def_type} ${_def})
      endif()
    endforeach()
  endif()

endfunction()

project(${_PROJ_NAME}) # -------------------

project_part(NAME json LINK nlohmann_json)
project_part(NAME math INCLUDE ${_PROJ_IMPL_DIR})

project_part(NAME convert_to INCLUDE ${utf8_conv_SOURCE_DIR}/include)
project_part(NAME hash INCLUDE ${xxhct_SOURCE_DIR})
project_part(
  NAME
  stable
  LINK
  hash
  convert_to
  INCLUDE
  ${_PROJ_IMPL_DIR})

project_part(NAME logger LINK stable)
project_part(NAME mem LINK stable)
project_part(NAME runtime_modules LINK mem logger)
project_part(NAME valve LINK runtime_modules math)
project_part(NAME netvars LINK valve json)
project_part(NAME gui_interface LINK stable)
project_part(
  NAME
  gui
  LINK
  gui_interface
  logger
  d3d9.lib
  RmlCore
  RmlDebugger
  DEFINE
  RMLUI_STATIC_LIB
  RMLUI_DIR=${rml_SOURCE_DIR})
project_part(NAME hooks LINK gui valve)

project_part(NAME gui_test TYPE EXECUTABLE FILES ${_PROJ_IMPL_DIR}/gui_test/main.cpp LINK hooks)


#[[
#-----------------

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

project(${_PROJ_NAME}_gui_test)

add_executable(${PROJECT_NAME} ${_PROJ_IMPL_DIR}/gui_test/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${_PROJ_NAME}) ]]
