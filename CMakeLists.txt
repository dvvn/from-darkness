cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off) 
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) 

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED on)

project(from-datkness CXX)

set(FD_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(FD_PROJECTS_DIR ${FD_ROOT_DIR}/projects)
set(FD_IMPL_DIR ${FD_ROOT_DIR}/impl)
set(FD_DIR ${FD_IMPL_DIR}/fd)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "")
    set(CMAKE_BUILD_TYPE ${CMAKE_CONFIGURATION_TYPES})
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
else()
    set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
endif()

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_RELEASE_POSTFIX _r)
set(CMAKE_RELWITHDEBINFO_POSTFIX _rd)
set(CMAKE_MINSIZEREL_POSTFIX _rm)

if(MSVC)
  # Whole program optimization
  if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(/GL)
    add_link_options(/LTCG)
    add_compile_options(/Qpar /Qvec-report:1 /Qpar-report:1)
  endif()  
  add_compile_options(/Zc:preprocessor /Zc:__cplusplus /Zc:lambda /permissive-)
  add_compile_options(/wd5103 /wd4291 /wd4250 /wd5104)
  add_compile_options(/MP${_CPUS} /EHsc)
  add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=0)

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

include(FetchContent)
set(GIT_SHALLOW true)
set(FETCHCONTENT_QUIET off)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/.deps1)

set(_GIT_DIR "C:/git" CACHE STRING "The repos storage")

include(ProcessorCount)
ProcessorCount(_CPUS)
message(DEBUG "Cpu's count: ${_CPUS}")

function(FetchContent_Load _owner _name)
    set(_path ${_owner}/${_name})
    set(_deps ${PROJECT_SOURCE_DIR}/.deps/${_path}/)
    set(_proj ${_name})

    set(_SOURCE_DIR "${_GIT_DIR}/${_path}/")
    set(_SUBBUILD_DIR "${_deps}/subbuild/")
    set(_BINARY_DIR "${_deps}/bin/")

    FetchContent_Declare(${_proj}        
        SOURCE_DIR ${_SOURCE_DIR}
        SUBBUILD_DIR ${_SUBBUILD_DIR}
        BINARY_DIR ${_BINARY_DIR}
        GIT_REPOSITORY "https://github.com/${_path}.git"
        ${ARGN}
    )    

    #[[
    if(EXISTS ${_SOURCE_DIR}/CMakeLists.txt)
        FetchContent_MakeAvailable(${_proj})
    else()
        FetchContent_Populate(${_proj})
    endif()
    ]]

    set(${_proj}_SOURCE_DIR ${_SOURCE_DIR} PARENT_SCOPE)
    set(${_proj}_SUBBUILD_DIR ${_SUBBUILD_DIR} PARENT_SCOPE)
    set(${_proj}_BINARY_DIR ${_BINARY_DIR} PARENT_SCOPE)
endfunction()

set(BOOST_RUNTIME_LINK static)
#set(BOOST_ENABLE_MPI ON)
FetchContent_Load(boostorg boost)
FetchContent_MakeAvailable(boost)

set(DISABLE_FORCE_DEBUG_POSTFIX on)
set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
FetchContent_Load(freetype freetype)
FetchContent_MakeAvailable(freetype)

FetchContent_Load(ocornut imgui GIT_TAG features/string_view)
FetchContent_Populate(imgui)

set(XXHASH_BUILD_XXHSUM off)
FetchContent_Load(Cyan4973 xxHash GIT_TAG dev)
FetchContent_Populate(xxHash)
add_subdirectory(${xxHash_SOURCE_DIR}/cmake_unofficial ${xxHash_BINARY_DIR})
target_compile_definitions(xxhash PUBLIC XXH_NO_LONG_LONG=1)

FetchContent_Load(ekpyron xxhashct)
FetchContent_Populate(xxhashct)

set(JSON_MultipleHeaders on)
set(JSON_DisableEnumSerialization on)
FetchContent_Load(nlohmann json)
FetchContent_MakeAvailable(json)

FetchContent_Load(stephenberry glaze GIT_TAG main)
FetchContent_MakeAvailable(glaze)
    
FetchContent_Load(martinus robin-hood-hashing)
FetchContent_MakeAvailable(robin-hood-hashing)

FetchContent_Load(max0x7ba atomic_queue)
FetchContent_Populate(atomic_queue)

set(FMT_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
FetchContent_Load(fmtlib fmt)
FetchContent_MakeAvailable(fmt)
target_compile_definitions(fmt PUBLIC FMT_USE_FULL_CACHE_DRAGONBOX=1)

set(SUBHOOK_STATIC on)
set(SUBHOOK_INSTALL off)
set(SUBHOOK_TESTS off)
FetchContent_Load(Zeex subhook)
FetchContent_MakeAvailable(subhook)

set(SPDLOG_FMT_EXTERNAL on)
set(SPDLOG_WCHAR_SUPPORT on)
set(SPDLOG_WCHAR_FILENAMES on)
FetchContent_Load(gabime spdlog GIT_TAG v1.x)
FetchContent_MakeAvailable(spdlog)
target_compile_definitions(spdlog PUBLIC basic_runtime=runtime_format_string)
set_target_properties(spdlog PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

set(SIGMATCH_DEV_MODE off)
set(SIGMATCH_BUILD_EXAMPLES off)
set(SIGMATCH_BUILD_BENCHMARKS off)
set(SIGMATCH_BUILD_TESTS off)
FetchContent_Load(SpriteOvO sigmatch GIT_TAG dev)
FetchContent_MakeAvailable(sigmatch)

FetchContent_Load(JustasMasiulis lazy_importer)
FetchContent_Populate(lazy_importer)

# -------------------

add_subdirectory(${FD_PROJECTS_DIR}/imgui ${imgui_BINARY_DIR})
add_subdirectory(${FD_PROJECTS_DIR}/shared)
add_subdirectory(${FD_PROJECTS_DIR}/gui_test)
add_subdirectory(${FD_PROJECTS_DIR}/dll)