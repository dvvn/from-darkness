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
    set(CMAKE_BUILD_TYPE ${CMAKE_CONFIGURATION_TYPES})
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
else()
    set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
endif()

set(_FD_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(_FD_IMPL_DIR ${_FD_ROOT_DIR}/impl)
set(_FD_OUT_DIR ${_FD_ROOT_DIR}/.out/${CMAKE_BUILD_TYPE})

set(_FD_SHARED_DIR ${_FD_IMPL_DIR}/shared)
set(_FD_ENTRY_DIR ${_FD_IMPL_DIR}/entry)

include(ProcessorCount)
ProcessorCount(_CPUS)
message(DEBUG "Cpu's count: ${_CPUS}")

if(MSVC)
  # Whole program optimization
  if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(/GL
        /Qpar /Qvec-report:1 /Qpar-report:1)
    add_link_options(/LTCG)
  else()
    
  endif()
  add_compile_options(
      /Zc:preprocessor /Zc:__cplusplus /Zc:lambda /permissive-
      /wd5103 /wd4291 /wd4250 /wd5104
      /MP${_CPUS} /EHsc)
  add_link_options(/SAFESEH:NO)
  add_compile_definitions(
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

include(FetchContent)
set(GIT_SHALLOW true)
set(FETCHCONTENT_QUIET off)
set(FETCHCONTENT_BASE_DIR ${_FD_ROOT_DIR}/.deps1)

set(_GIT_DIR "C:/git" CACHE STRING "The repos storage")

set(_FD_DEPS)
set(_FD_DEPS_INCLUDE)
set(_FD_DEPS_DIRS)

string(TIMESTAMP _current_time "%d.%m.%Y")
macro(FetchContent_Activate _name)
    if(DEFINED ${_name}_fetch AND ${_name}_fetch)
        set(_previous_time ${${_name}_fetch})
    else()
        set(_previous_time 0)
    endif()

    if(${_current_time} STREQUAL ${_previous_time})
        set(FETCHCONTENT_FULLY_DISCONNECTED on)
    else()
        set(FETCHCONTENT_FULLY_DISCONNECTED off)
        set(${_name}_fetch_tmp ${_current_time})
    endif()
endmacro()

macro(FetchContent_Activate_Save _name)
    if(DEFINED ${_name}_fetch_tmp)
        FetchContent_GetProperties(${_name})
        string(TOLOWER ${_name} _name_lower)
        if(${_name_lower}_POPULATED)
            set(${_name}_fetch ${${_name}_fetch_tmp} CACHE INTERNAL "" FORCE)
        endif()
    endif()
endmacro()

macro(Dependency_Load _owner _name)
    set(_path ${_owner}/${_name})
    set(_deps ${_FD_ROOT_DIR}/.deps/${_path})

    set(_source_dir "${_GIT_DIR}/${_path}")
    set(_subbuild_dir "${_deps}/subbuild")
    set(_binary_dir "${_deps}/bin")

    FetchContent_Activate(${_name})
    FetchContent_Declare(${_name}        
        SOURCE_DIR ${_source_dir}
        SUBBUILD_DIR ${_subbuild_dir}
        BINARY_DIR ${_binary_dir}
        GIT_REPOSITORY https://github.com/${_path}.git
        GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
        ${ARGN}
    )

    set(${_name}_SOURCE_DIR ${_source_dir})
    set(${_name}_SUBBUILD_DIR ${_subbuild_dir})
    set(${_name}_BINARY_DIR ${_binary_dir})

    list(APPEND _FD_DEPS_DIRS ${_source_dir})
endmacro()

macro(Dependency_Use _name)
    FetchContent_MakeAvailable(${_name})
    FetchContent_Activate_Save(${_name})

    if(TARGET ${_name}::${_name})
        set(_target ${_name}::${_name})
    elseif(TARGET ${_name})
        set(_target ${_name})
    endif()

    if(_target)
        list(APPEND _FD_DEPS ${_target})
        message(DEBUG "target ${_target} added")
    else()
        message(DEBUG "${_name} isn't a target name")
    endif()
endmacro()

macro(Dependency_Use_Boost)
    FetchContent_MakeAvailable(boost)
    FetchContent_Activate_Save(boost)

    set(_boost_libs ${BOOST_INCLUDE_LIBRARIES})
    list(TRANSFORM _boost_libs PREPEND Boost::)

    foreach(_lib ${_boost_libs})
         message(DEBUG "target ${_lib} added")
    endforeach()

    list(APPEND _FD_DEPS ${_boost_libs})
endmacro()

macro(Dependency_Use_Imgui)
    if(EXISTS "${imgui_SOURCE_DIR}/CMakeLists.txt")
        Dependency_Use(imgui)
    else()
        FetchContent_Populate(imgui)
        FetchContent_Activate_Save(imgui)

        file(GLOB _IMGUI_FILES ${imgui_SOURCE_DIR}/*.cpp)
        if(TARGET freetype)
            list(APPEND _IMGUI_FILES
                ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
            )
        endif()
        
        list(APPEND _IMGUI_FILES
            ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
        )      
        add_library(imgui ${_IMGUI_FILES})
        add_library(imgui::imgui ALIAS imgui)             

        target_sources(imgui PUBLIC
            ${imgui_SOURCE_DIR}/misc/debuggers/imgui.natvis
            ${imgui_SOURCE_DIR}/misc/debuggers/imgui.natstepfilter
        )

        target_compile_definitions(imgui PRIVATE 
            IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
            IMGUI_USE_BGRA_PACKED_COLOR #d3d9 only
        )
        
        if(TARGET freetype)
            target_link_libraries(imgui PUBLIC freetype)
            target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)
        endif()
        target_include_directories(imgui PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
        )

        list(APPEND _FD_DEPS "imgui::imgui")
        message(DEBUG "target imgui::imgui added")
    endif()
endmacro()

macro(Dependency_Populate _name)
    FetchContent_Populate(${_name})
    FetchContent_Activate_Save(${_name})

    set(_src_dir ${${_name}_SOURCE_DIR})

    if(EXISTS ${_src_dir}/include)
        set(_include ${_src_dir}/include)
        set(_info ${_name}/include)
    else()
        set(_include ${_src_dir})
        set(_info ${_name}/)
    endif()

    message(DEBUG "${_info} directory included")
    list(APPEND _FD_DEPS_INCLUDE ${_include})
endmacro()

set(BOOST_INCLUDE_LIBRARIES
    #thread
    algorithm
    container
    container_hash
    filesystem
    #json
    sort
    io
    geometry
    qvm
    fusion
    hana
    static_string
    unordered
    nowide
    lambda2)
set(BOOST_RUNTIME_LINK static)
#set(BOOST_ENABLE_MPI ON)
Dependency_Load(boostorg boost)
Dependency_Use_Boost()

#[[
set(XXHASH_BUILD_XXHSUM off)
Dependency_Load(Cyan4973 xxHash GIT_TAG dev)
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
#Dependency_Load(stephenberry glaze GIT_TAG main)
#Dependency_Use(glaze)

#Dependency_Load(martinus robin-hood-hashing)
#Dependency_Use(robin-hood-hashing)

#Dependency_Load(max0x7ba atomic_queue)
#Dependency_Populate(atomic_queue)

Dependency_Load(fmtlib fmt)
Dependency_Use(fmt)
target_compile_definitions(fmt PUBLIC FMT_USE_FULL_CACHE_DRAGONBOX=1)

#[[
set(SUBHOOK_STATIC on)
set(SUBHOOK_INSTALL off)
set(SUBHOOK_TESTS off)
Dependency_Load(Zeex subhook)
Dependency_Use(subhook)
]]

#Dependency_Load(m417z minhook GIT_TAG multihook)
Dependency_Load(TsudaKageyu minhook)
Dependency_Use(minhook)

Dependency_Load(Neargye magic_enum)
Dependency_Use(magic_enum)

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
Dependency_Load(gabime spdlog GIT_TAG v1.x)
Dependency_Use(spdlog)
target_compile_definitions(spdlog PUBLIC basic_runtime=runtime_format_string)
]]

#[[
set(SIGMATCH_DEV_MODE off)
set(SIGMATCH_BUILD_EXAMPLES off)
set(SIGMATCH_BUILD_BENCHMARKS off)
set(SIGMATCH_BUILD_TESTS off)
Dependency_Load(SpriteOvO sigmatch GIT_TAG dev)
Dependency_Use(sigmatch)
]]

#Dependency_Load(JustasMasiulis lazy_importer)
#Dependency_Populate(lazy_importer)

Dependency_Load(danielkrupinski x86RetSpoof)
Dependency_Populate(x86RetSpoof)

set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
Dependency_Load(freetype freetype)
Dependency_Use(freetype)

Dependency_Load(ocornut imgui GIT_TAG features/string_view)
Dependency_Use_Imgui()
target_compile_definitions(imgui PUBLIC
    IMGUI_USER_CONFIG="${_FD_SHARED_DIR}/render/config.h")

# -------------------

set(_nat_name fd_deps)
foreach(_lib ${_FD_DEPS_DIRS})
    string(REPLACE "/" "\\" _natjmc_lib \"${_lib}*\")
    set(_NATJMC_DATA "${_NATJMC_DATA}<File Name=${_natjmc_lib} />")
endforeach()
configure_file(${_FD_IMPL_DIR}/${_nat_name}_natjmc ${_FD_IMPL_DIR}/${_nat_name}.natjmc)
#configure_file(${_FD_IMPL_DIR}/${_nat_name}_natstepfilter ${_FD_IMPL_DIR}/${_nat_name}.natstepfilter)

file(GLOB_RECURSE _shared_src ${_FD_SHARED_DIR}/*.cpp)
file(GLOB_RECURSE _visualizers ${_FD_IMPL_DIR}/*.nat*)
add_library(FD ${_shared_src} ${_visualizers})
target_link_libraries(FD PUBLIC ${_FD_DEPS})
target_include_directories(FD PUBLIC ${_FD_SHARED_DIR}/ ${_FD_DEPS_INCLUDE})

add_library(FDLIB SHARED ${_FD_ENTRY_DIR}/library.cpp)
target_link_libraries(FDLIB PUBLIC FD)
add_executable(FDLIB_GUI ${_FD_ENTRY_DIR}/gui.cpp)
target_link_libraries(FDLIB_GUI PUBLIC FD d3d9.lib)