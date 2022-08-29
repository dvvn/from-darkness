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

# if(MSVC) idk, not defined
add_compile_options(/Zc:preprocessor /Zc:__cplusplus)
add_compile_options(/wd5103 /wd4291 /wd4250)
add_compile_options(/MP${_CPUS} /EHsc)
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

FetchContent_Declare(veque GIT_REPOSITORY https://github.com/Shmoopty/veque.git)
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

function(target_include_directory _target _type _dir)
  target_include_directories(${_target} ${_type} ${_dir})
  message(DEBUG "${_target}: directory \"${_dir}\" included (${_type})")
endfunction()

function(target_link_library _target _type _lib)
  target_link_libraries(${_target} ${_type} ${_lib})
  message(DEBUG "${_target}: library \"${_lib}\" linked (${_type})")
endfunction()

function(target_compile_definition _target _type _def)
  target_compile_definitions(${_target} ${_type} ${_def})
  message(DEBUG "${_target}: macro \"${_def}\" added (${_type})")
endfunction()

function(add_subproject _name)
  cmake_parse_arguments(PART "" "TYPE" "FILES;INCLUDE;LINK;DEFINE" ${ARGN})

  #[[ if(NOT PART_NAME)
    set(_name ${PART_NAME})
  else()
    message(FATAL_ERROR "Name not set!")
  endif() ]]

  set(_VIS_MODES "PRIVATE|PUBLIC|INTERFACE")

  #---------------

  if(NOT PART_FILES)
    set(_find_default_files true)
  else()
    #message(DEBUG "parsed files: ${PART_FILES}")
    foreach(_item ${PART_FILES})
      if(_item MATCHES "FILTER=*")
        # multiple filters allowed
        #[[ if(DEFINED _files_filter)
          message(FATAL_ERROR "files filter already set!")
        endif() ]]
        string(REPLACE "FILTER=" "" _files_filter ${_item})
        #message(DEBUG "filter set to \"${_files_filter}\"")
      elseif(IS_DIRECTORY ${_item})
        #message(DEBUG "found directory: ${_item}")
        file(GLOB_RECURSE _files_tmp ${_item}/*.ixx ${_item}/*.cpp)
      elseif(EXISTS ${_item})
        list(APPEND _files_tmp ${_item})
        #message(DEBUG "found file: ${_item}")
      else() #maybe it contains regex (TODO: find better way to check)
        file(GLOB_RECURSE _files_tmp ${_item})
        list(APPEND _files ${_files_tmp})
        #message(DEBUG "files found for ${_item}: ${_files_tmp}")
      endif()

      if(DEFINED _files_tmp)
        if(DEFINED _files_filter)
          list(FILTER _files_tmp EXCLUDE REGEX ${_files_filter})
        endif()
        list(APPEND _files ${_files_tmp})
        unset(_files_tmp)
      endif()
    endforeach()

    list(LENGTH PART_FILES _items_count)
    if(_items_count EQUAL 1 AND DEFINED _files_filter) #the only filter detected
      set(_find_default_files true)
    else()
      set(_find_default_files false)
    endif()
    #message(DEBUG "find default files set to '${_find_default_files}', items count: ${_items_count}, filter: ${_files_filter}")
  endif()

  set(_add_prefix_to_name true)
  if(_find_default_files)
    set(_search_dir ${_PROJ_IMPL_DIR}/*/${_name})
    set(_search_dir2 ${_PROJ_IMPL_DIR}/${_name})
    file(GLOB_RECURSE _files ${_search_dir}/*.ixx ${_search_dir}/*.cpp ${_search_dir2}/*.ixx ${_search_dir2}/*.cpp)
    #TODO: check are only one root dir exsists
    if(DEFINED _files_filter)
      list(FILTER _files EXCLUDE REGEX ${_files_filter})
    endif()
  else()
    foreach(_file ${_files})
      if(_add_prefix_to_name AND NOT ${_file} MATCHES ${_PROJ_IMPL_DIR}/*)
        set(_add_prefix_to_name false)
      endif()
    endforeach()
  endif()

  #message(DEBUG "files found: ${_files} (default: ${_find_default_files})")
  list(LENGTH _files _files_count)
  if(_files_count EQUAL 0)
    if(PART_TYPE AND NOT PART_TYPE STREQUAL "INTERFACE")
      message(FATAL_ERROR "Incorrect TYPE!")
    endif()
    set(_type INTERFACE)
  else()
    if(PART_TYPE)
      set(_type ${PART_TYPE})
    else()
      set(_type STATIC)
    endif()
  endif()

  if(_add_prefix_to_name)
    set(_target ${_PROJ_NAME}_${_name})
  else()
    set(_target ${_name})
  endif()

  if(_type STREQUAL "EXECUTABLE")
    add_executable(${_target} ${_files})
    set(_link_type_root PRIVATE)
  else()
    add_library(${_target} ${_type} ${_files})
    if(_type STREQUAL "INTERFACE") #ifcSearchDir
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
      elseif(_item MATCHES ${_VIS_MODES})
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
      if(_lib MATCHES ${_VIS_MODES})
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
      if(_def MATCHES ${_VIS_MODES})
        set(_def_type ${_def})
      else()
        target_compile_definition(${_target} ${_def_type} ${_def})
      endif()
    endforeach()
  endif()

endfunction()

# -------------------

set(_DEFAULT_FILES_FILTER "/\_+")

file(GLOB _IMGUI_CORE_FILES ${imgui_SOURCE_DIR}/*.cpp)

add_subproject(
  imgui
  LINK
  PRIVATE
  freetype
  INCLUDE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  PRIVATE
  ${imgui_SOURCE_DIR}/misc/freetype
  FILES
  ${_IMGUI_CORE_FILES}
  ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
  DEFINE
  IMGUI_ENABLE_FREETYPE
  IMGUI_USE_WCHAR32
  IMGUI_DEFINE_MATH_OPERATORS
  IMGUI_DISABLE_OBSOLETE_FUNCTIONS
  IMGUI_DISABLE_OBSOLETE_KEYIO
  IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
  ImDrawIdx=size_t)

add_subproject(functional LINK function2 INCLUDE ${_PROJ_IMPL_DIR})

add_subproject(
  stable
  LINK
  nlohmann_json
  functional
  fmt
  INCLUDE
  ${_PROJ_IMPL_DIR}
  ${utf8_conv_SOURCE_DIR}/include
  ${veque_SOURCE_DIR}/include
  ${xxhct_SOURCE_DIR})

add_subproject(logger LINK stable)
add_subproject(mem LINK stable)
add_subproject(rt_modules LINK mem logger)
add_subproject(math INCLUDE ${_PROJ_IMPL_DIR})
add_subproject(valve_interfaces LINK rt_modules math)
add_subproject(
  netvars
  LINK
  valve_interfaces
  FILES
  FILTER=${_DEFAULT_FILES_FILTER}
 #[[  ${_PROJ_IMPL_DIR}/netvars
  ${_PROJ_IMPL_DIR}/valve_custom/base_entity.ixx ]]
  DEFINE
  PRIVATE
  ${_PROJ_NAME_UPPER}_WORK_DIR=${_PROJ_WORK_DIR}
  ${_PROJ_NAME_UPPER}_ROOT_DIR=${_PROJ_IMPL_DIR})
add_subproject(valve_custom LINK netvars)
add_subproject(gui_interface LINK stable)
add_subproject(
  gui
  LINK
  gui_interface
  logger
  d3d9.lib
  imgui
  FILES
  FILTER=${_DEFAULT_FILES_FILTER}) #[[ RmlCore  RmlDebugger  DEFINE  RMLUI_STATIC_LIB  RMLUI_DIR=${rml_SOURCE_DIR} ]]
add_subproject(hooks LINK subhook gui valve_custom)

add_subproject(gui_test TYPE EXECUTABLE LINK hooks)
add_subproject(library TYPE SHARED LINK hooks)
