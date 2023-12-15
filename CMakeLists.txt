cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off) 
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) 

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED on)

#set(CMAKE_EXPORT_COMPILE_COMMANDS on)
#set(CMAKE_VERBOSE_MAKEFILE on)

project(from-datkness CXX)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "")
    list(LENGTH CMAKE_CONFIGURATION_TYPES _cfgs_count)
    if(NOT _cfgs_count EQUAL 1)
        message(FATAL_ERROR "Unsupported CMAKE_CONFIGURATION_TYPES")
    endif()
    set(CMAKE_BUILD_TYPE ${CMAKE_CONFIGURATION_TYPES})
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
elseif(NOT CMAKE_MESSAGE_LOG_LEVEL)
    set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
endif()

set(_FD_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(_FD_IMPL_DIR ${_FD_ROOT_DIR}/impl)
set(_FD_OUT_DIR ${_FD_ROOT_DIR}/.out)

set(_FD_TEMPLATE_DIR ${_FD_IMPL_DIR}/template)
set(_FD_SHARED_DIR ${_FD_IMPL_DIR}/shared)
set(_FD_EXTERNAL_DIR ${_FD_IMPL_DIR}/external)
set(_FD_TESTS_DIR ${_FD_IMPL_DIR}/tests)

set(_FD_PROJECT_DIR ${_FD_IMPL_DIR}/${_FD_TARGET})
set(_FD_PROJECT_IMPL_DIR ${_FD_PROJECT_DIR}/IMPL)

find_package(Git)
if(NOT Git_FOUND)
  message(FATAL_ERROR "Git not found")
endif()

include(ProcessorCount)
ProcessorCount(_CPUS)
message(DEBUG "Cpu's count: ${_CPUS}")

if(MSVC)
  # Whole program optimization
  add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/GL /Qpar /Qvec-report:1 /Qpar-report:1>")
  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/LTCG>")
    
  add_compile_options(
      /Zc:__STDC__ /Zc:__cplusplus
      /Zc:preprocessor /Zc:lambda /permissive- /Zc:inline /Zc:templateScope /Zc:threadSafeInit-
      /wd5103 /wd4291 /wd4250 /wd5104
      /MP${_CPUS}
      /EHsc
      #/GR- #disable RTTI
  )
  add_link_options(/SAFESEH:NO)
  add_compile_definitions(
      _HAS_EXCEPTIONS=0 #disable exceptions
      UNICODE
      _UNICODE
      NOMINMAX
      WIN32_LEAN_AND_MEAN
      MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=0)

  string(REPLACE "Zi" "ZI" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  string(REPLACE "Zi" "ZI" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

  #set(CMAKE_SHARED_LINKER_FLAGS_DEBUG ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} "/SAFESEH:NO")
  #set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG "/SAFESEH:NO")
  #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")

  # Force enable a good many optimizations
  # Aggresive inlining (Release)
  string(REPLACE "Ob2" "Ob3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  # Aggresive inlining (RelWithDebInfo)
  string(REPLACE "Ob1" "Ob3" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  string(REPLACE "O1" "O2" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    # For all objects:
  # - strip unused functions
  # - fold identical functions
  # - link-time code generation
  string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
  string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
  string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /OPT:ICF,REF")
  set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /OPT:ICF,REF")
  set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} /OPT:ICF,REF")
endif()

#------------------------

set(_FD_DEPS)
macro(FD_append_dep)
    foreach(_dep_append ${ARGN})
        list(APPEND _FD_DEPS ${_dep_append})
        if(TARGET ${_dep_append})
            message(DEBUG "target ${_dep_append} added")
        endif()
    endforeach()
endmacro()

set(_FD_DEPS_INCLUDE)
macro(FD_append_dep_include _inc_append)
    list(APPEND _FD_DEPS_INCLUDE ${_inc_append})
    message(DEBUG "${_inc_append} directory included") 
endmacro()

set(_FD_DEPS_DIRS)
macro(FD_append_dep_dir _dir_append)
    list(APPEND _FD_DEPS_DIRS ${_dir_append})
endmacro()

#todo: current drive name
set(_GIT_DIR "C:/Dev/Git" CACHE STRING "The repos storage")
macro(Run_git _work_dir)
    set(_argn ${ARGN})
    execute_process(
            COMMAND             ${GIT_EXECUTABLE} ${ARGN}
            WORKING_DIRECTORY   ${_work_dir}
            RESULT_VARIABLE     _git_result
            ECHO_ERROR_VARIABLE
    )
    if(_git_result EQUAL 0)
        set(_GIT_ERROR false #[[PARENT_SCOPE]])
    else()
        set(_GIT_ERROR true #[[PARENT_SCOPE]])
    endif()
    if(_git_result STREQUAL "")
        set(_GIT_RESULT "UNKNOWN" #[[PARENT_SCOPE]])
    else()
        set(_GIT_RESULT ${_git_result} #[[PARENT_SCOPE]])
    endif()
endmacro()

macro(Git_error)
    if(NOT DEFINED _GIT_RESULT)
        message(FATAL_ERROR "_GIT_RESULT not set!")
    else()
        message(FATAL_ERROR "git error: ${_GIT_RESULT}")
    endif()
endmacro()

macro(Check_git_error)
    if(NOT DEFINED _GIT_ERROR)
        message(FATAL_ERROR "_GIT_ERROR not set!")
    elseif(_GIT_ERROR)
        Git_error()
    endif()
endmacro()

macro(Check_directory_empty _check_dir)
    set(_DIR_EMPTY true #[[PARENT_SCOPE]])
    if(EXISTS ${_check_dir})
        file(GLOB _items ${_check_dir}/*)
        list(LENGTH _items _items_count)
        if(_items_count)
            set(_DIR_EMPTY false #[[PARENT_SCOPE]])       
        endif()
    endif()
endmacro()

macro(Set_variable _var_name _var_postfix _default_value)    
    string(TOUPPER ${_var_postfix} _var_postfix_UP)
    set(_var_name_full ${_var_name}_${_var_postfix_UP})
    set(_out_var _${_var_postfix})
    if(NOT DEFINED ${_var_name_full})
        set(${_out_var} ${_default_value})
    else()
        set(_stored_value ${${_var_name_full}})
        while(DEFINED ${_stored_value})
            set(_stored_value ${${_stored_value}})
        endwhile()
        set(${_out_var} ${_stored_value})
    endif()
endmacro()

set(_ADD_TARGET 2) #load cmake, add created targets
set(_ADD_TARGET_DEP 1) #load cmake
set(_ADD_INCLUDE 1)

macro(Set_timestamp)
    if(NOT DEFINED _TIMESTAMP)
        string(TIMESTAMP _TIMESTAMP "%d.%m.%Y")
    endif()
endmacro()

macro(Check_connection)
    if(NOT DEFINED _NO_CONNECTION)
        execute_process(
            COMMAND ping www.google.com -n 2 -w 1000
            RESULT_VARIABLE _NO_CONNECTION)
    endif()
endmacro()

set(_GIT_ARGS --verbose --jobs=${_CPUS} --depth=1 --recurse-submodules --no-tags)
macro(Dependency_Load _owner _name)
    set(_git_branch ${ARGN})
    if(NOT _git_branch)
        set(_git_branch master)
    endif()
    set(_last_timestamp_path ${_FD_OUT_DIR}/git/${_owner}/${_name}/${_git_branch}/timestamp.txt)   
    set(_git_update true)
    
    set(_subdir ${_owner}/${_name})
    set(_source_dir ${_GIT_DIR}/${_subdir})
    set(_binary_dir ${CMAKE_BINARY_DIR}/${_subdir})
    set(_url https://github.com/${_subdir}.git)

    set(${_name}_SOURCE_DIR ${_source_dir} #[[PARENT_SCOPE]])       

    Check_directory_empty(${_source_dir})
    if(_DIR_EMPTY)
        Run_git(${_GIT_DIR} clone --branch ${_git_branch} ${_GIT_ARGS} --shallow-submodules --single-branch ${_url} ${_subdir})
        if(_GIT_ERROR)
            file(REMOVE_RECURSE ${_source_dir})
            Git_error()
        endif()
        Set_timestamp()
    else()
        if(EXISTS ${_last_timestamp_path})
            file(READ ${_last_timestamp_path} _last_timestamp)
            Set_timestamp()
            if(_TIMESTAMP EQUAL _last_timestamp)
                set(_git_update false)
            else()
                Check_connection()
                if(_NO_CONNECTION)
                    set(_git_update false)
                endif()
            endif()
        endif()

        if(_git_update)
            #Run_git(${_source_dir} pull ${_GIT_ARGS})
            Run_git(${_source_dir} fetch ${_GIT_ARGS})
            Check_git_error()
            Run_git(${_source_dir} reset --hard origin/${_git_branch})
            Check_git_error()
            Run_git(${_source_dir} clean -dfx)
            Check_git_error()
        endif()
    endif()
        
    if(_git_update)
        file(WRITE ${_last_timestamp_path} ${_TIMESTAMP})
    endif()
       
    if(NOT EXISTS ${_source_dir}/CMakeLists.txt)
        set(_add_target 0)
    else()
        Set_variable(${_name} add_target ${_ADD_TARGET})

        if(_add_target GREATER 0)
            add_subdirectory(${_source_dir} ${_binary_dir})
        endif()

        if(_add_target GREATER 1)
            if(TARGET ${_name}::${_name})
                FD_append_dep(${_name}::${_name})
            elseif(TARGET ${_name})
                FD_append_dep(${_name})
            else()
                message(FATAL_ERROR "${_subdir}: no targets found!")
            endif()
        endif()
    endif()

    if(_add_target GREATER 1)
        set(_add_include false)
    else()
        Set_variable(${_name} add_include ${_ADD_INCLUDE})
    endif()

    if(_add_include)
        if(EXISTS ${_source_dir}/include)
            FD_append_dep_include(${_source_dir}/include)
        else()
            FD_append_dep_include(${_source_dir})
        endif()
    endif()

    FD_append_dep_dir(${_source_dir})
endmacro()

macro(Dependency_Load_Boost)
    list(APPEND BOOST_INCLUDE_LIBRARIES ${ARGN})

    set(boost_ADD_TARGET _ADD_TARGET_DEP)
    set(boost_ADD_INCLUDE _ADD_INCLUDE)
    Dependency_Load(boostorg boost)

    set(_boost_libs ${BOOST_INCLUDE_LIBRARIES})
    list(TRANSFORM _boost_libs PREPEND Boost::)

    FD_append_dep(${_boost_libs})     
endmacro()

macro(Dependency_Load_Imgui)
    set(_imgui_load_args features/string_view)
    if(EXISTS ${imgui_SOURCE_DIR}/CMakeLists.txt)         
        Dependency_Load(ocornut imgui ${_imgui_load_args})
    else()
        set(imgui_ADD_TARGET false)
        set(imgui_ADD_INCLUDE false)
        Dependency_Load(ocornut imgui ${_imgui_load_args})

        set(_backends ${imgui_SOURCE_DIR}/backends)

        set(_include_dirs PUBLIC ${imgui_SOURCE_DIR} ${_backends})
        file(GLOB _sources ${imgui_SOURCE_DIR}/*.cpp)
        list(APPEND _sources ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp)  
        file(GLOB _nat_files ${imgui_SOURCE_DIR}/misc/debuggers/*.nat*)
        set(_compile_defs PRIVATE IMGUI_IMPL_WIN32_DISABLE_GAMEPAD)

        if(${_FD_TARGET} STREQUAL CSGO)
            list(APPEND _sources ${_backends}/imgui_impl_dx9.cpp)
            list(APPEND _compile_defs PUBLIC IMGUI_USE_BGRA_PACKED_COLOR)
        elseif(${_FD_TARGET} STREQUAL CS2)
            list(APPEND _sources ${_backends}/imgui_impl_dx11.cpp)  
        endif()

        if(TARGET freetype)
            list(APPEND _sources ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp)
            list(APPEND _compile_defs PUBLIC IMGUI_ENABLE_FREETYPE)
            list(APPEND _link_libs PUBLIC freetype)
        endif()   
                              
        add_library(imgui ${_sources} ${_nat_files})
        add_library(imgui::imgui ALIAS imgui)     

        target_compile_definitions(imgui ${_compile_defs})
        if(DEFINED _link_libs)
            target_link_libraries(imgui ${_link_libs})
        endif()
        target_include_directories(imgui ${_include_dirs})
        
        FD_append_dep("imgui::imgui")
    endif()

    set(_imgui_config_name imgui_config.h)
    if(NOT EXISTS ${_FD_EXTERNAL_DIR}/${_imgui_config_name})
        message(FATAL_ERROR "imgui custom config not found!")
    endif()
    target_include_directories(imgui PUBLIC ${_FD_EXTERNAL_DIR})
    target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="${_imgui_config_name}")
endmacro()

macro(Load_SHARED)
    set(_nat_name fd_deps)
    foreach(_lib ${_FD_DEPS_DIRS})
        string(REPLACE "/" "\\" _natjmc_lib \"${_lib}*\")
        set(_NATJMC_DATA "${_NATJMC_DATA}<File Name=${_natjmc_lib} />")
    endforeach()

    set(_nat_out_dir ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    configure_file(${_FD_TEMPLATE_DIR}/${_nat_name}_natjmc ${_nat_out_dir}/${_nat_name}.natjmc)
    #configure_file(${_FD_TEMPLATE_DIR}/${_nat_name}_natstepfilter ${_nat_out_dir}/${_nat_name}.natstepfilter)

    file(GLOB _global_nat ${_nat_out_dir}/*.nat*)    
    file(GLOB_RECURSE _shared_src ${_FD_SHARED_DIR}/*.cpp)
    file(GLOB_RECURSE _shared_nat ${_FD_SHARED_DIR}/*.nat*)

    add_library(FD_SHARED ${_global_nat} ${_shared_src} ${_shared_nat})
    target_link_libraries(FD_SHARED PUBLIC ${_FD_DEPS})
    target_include_directories(FD_SHARED PUBLIC ${_FD_SHARED_DIR} ${_FD_EXTERNAL_DIR} ${_FD_DEPS_INCLUDE})
    
    file(GLOB_RECURSE _project_src ${_FD_PROJECT_IMPL_DIR}/*.cpp)
    file(GLOB_RECURSE _project_nat ${_FD_PROJECT_IMPL_DIR}/*.nat*)
    set(_project_add ${_project_src} ${_project_nat})

    set(_FD_SHARED_TARGET FD_${_FD_TARGET}_IMPL)
    list(LENGTH _project_add _project_add_count)
    if(_project_add_count EQUAL 0)
        add_library(${_FD_SHARED_TARGET} INTERFACE)
        target_link_libraries(${_FD_SHARED_TARGET} INTERFACE FD_SHARED)
        target_include_directories(${_FD_SHARED_TARGET} INTERFACE ${_FD_PROJECT_IMPL_DIR})
    else()
        add_library(${_FD_SHARED_TARGET} ${_project_add})
        target_link_libraries(${_FD_SHARED_TARGET} PUBLIC FD_SHARED)
        target_include_directories(${_FD_SHARED_TARGET} PUBLIC ${_FD_PROJECT_IMPL_DIR})
    endif()
endmacro()

set(BOOST_RUNTIME_LINK static)
#set(BOOST_ENABLE_MPI ON)
Dependency_Load_Boost(
    #thread
    #algorithm
    container
    #filesystem
    #json
    #sort
    #io
    #geometry
    #qvm
    #fusion
    hana
    mp11
    #static_string
    #unordered
    nowide
    lambda2
    #vmd
    preprocessor
)

#[[
set(XXHASH_BUILD_XXHSUM off)
Dependency_Load(Cyan4973 xxHash dev)
Dependency_Populate(xxHash)
add_subdirectory(${xxHash_SOURCE_DIR}/cmake_unofficial ${xxHash_BINARY_DIR})
target_compile_definitions(xxhash PUBLIC XXH_NO_LONG_LONG=1)

Dependency_Load(ekpyron xxhashct)
Dependency_Populate(xxhashct)
]]

#[[
set(JSON_MultipleHeaders on)
set(JSON_DisableEnumSerialization on)
Dependency_Load(nlohmann json)
Dependency_Use(json)
]]

#unable to use it now, because it change compiler flags and break the project
#Dependency_Load(stephenberry glaze main)
#Dependency_Use(glaze)

#Dependency_Load(martinus robin-hood-hashing)
#Dependency_Use(robin-hood-hashing)

#Dependency_Load(max0x7ba atomic_queue)
#Dependency_Populate(atomic_queue)

Dependency_Load(fmtlib fmt)
target_compile_definitions(fmt PUBLIC FMT_USE_FULL_CACHE_DRAGONBOX=$<NOT:$<CONFIG:Debug>>)

#[[
set(SUBHOOK_STATIC on)
set(SUBHOOK_INSTALL off)
set(SUBHOOK_TESTS off)
Dependency_Load(Zeex subhook)
Dependency_Use(subhook)
]]

#Dependency_Load(m417z minhook multihook)
Dependency_Load(TsudaKageyu minhook)

#Dependency_Load(Neargye magic_enum)

#[[
set(SPDLOG_BUILD_EXAMPLE off)
set(SPDLOG_INSTALL off)
set(SPDLOG_FMT_EXTERNAL on)
set(SPDLOG_NO_EXCEPTIONS on)
set(SPDLOG_WCHAR_SUPPORT on)
set(SPDLOG_WCHAR_FILENAMES on)
set(SPDLOG_NO_ATOMIC_LEVELS on)
set(SPDLOG_NO_TLS on)
set(SPDLOG_NO_THREAD_ID on)
Dependency_Load(gabime spdlog v1.x)
Dependency_Use(spdlog)
target_compile_definitions(spdlog PUBLIC basic_runtime=runtime_format_string)
]]

#[[
set(SIGMATCH_DEV_MODE off)
set(SIGMATCH_BUILD_EXAMPLES off)
set(SIGMATCH_BUILD_BENCHMARKS off)
set(SIGMATCH_BUILD_TESTS off)
Dependency_Load(SpriteOvO sigmatch dev)
Dependency_Use(sigmatch)
]]

#Dependency_Load(JustasMasiulis lazy_importer)
#Dependency_Populate(lazy_importer)

if(${_FD_TARGET} STREQUAL CSGO)
    set(x86RetSpoof_ADD_TARGET 0)
    Dependency_Load(danielkrupinski x86RetSpoof)
endif()

set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
set(freetype_ADD_TARGET _ADD_TARGET_DEP)
set(freetype_ADD_INCLUDE 0)
Dependency_Load(freetype freetype)

Dependency_Load_Imgui()

# -------------------

Load_SHARED()

if(EXISTS ${_FD_TESTS_DIR}/main.cpp)

else()
file(GLOB_RECURSE _tests_src ${_FD_TESTS_DIR}/*.cpp)
add_library(FD_TESTS ${_tests_src})
target_link_libraries(FD_TESTS PUBLIC ${_FD_SHARED_TARGET})
endif()

macro(Finish_library _name)
    target_link_libraries(${_name} PUBLIC ${_FD_SHARED_TARGET} FD_TESTS)
    target_include_directories(${_name} PUBLIC ${_FD_PROJECT_DIR} ${_FD_TEMPLATE_DIR})
endmacro()


if(EXISTS ${_FD_PROJECT_DIR}/main.cpp)
    add_library(FD_${_FD_TARGET} SHARED ${_FD_PROJECT_DIR}/main.cpp)
    Finish_library(FD_${_FD_TARGET})
endif()
if(EXISTS ${_FD_PROJECT_DIR}/gui.cpp)
    add_executable(FD_${_FD_TARGET}_GUI ${_FD_PROJECT_DIR}/gui.cpp)
    Finish_library(FD_${_FD_TARGET}_GUI)
endif()