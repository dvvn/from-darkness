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

set(FD_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(FD_IMPL_DIR ${FD_ROOT_DIR}/impl)
set(FD_DIR ${FD_IMPL_DIR}/fd)
set(FD_OUT_DIR ${FD_ROOT_DIR}/.out/${CMAKE_BUILD_TYPE})

#set(CMAKE_CXX_LINK_OPTIONS_IPO "")
#set(CMAKE_CXX_COMPILE_OPTIONS_IPO "")
#set(CMAKE_CXX_CREATE_STATIC_LIBRARY_IPO "")

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
set(FETCHCONTENT_BASE_DIR ${FD_ROOT_DIR}/.deps1)
#set(FETCHCONTENT_FULLY_DISCONNECTED on)

set(_GIT_DIR "C:/git" CACHE STRING "The repos storage")

function(FetchContent_Load _owner _name)
    set(_path ${_owner}/${_name})
    set(_deps ${FD_ROOT_DIR}/.deps/${_path}/)
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

set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
fetchContent_Load(freetype freetype)
fetchContent_MakeAvailable(freetype)

fetchContent_Load(ocornut imgui GIT_TAG features/string_view)
fetchContent_Populate(imgui)

#[[
set(XXHASH_BUILD_XXHSUM off)
FetchContent_Load(Cyan4973 xxHash GIT_TAG dev)
FetchContent_Populate(xxHash)
add_subdirectory(${xxHash_SOURCE_DIR}/cmake_unofficial ${xxHash_BINARY_DIR})
target_compile_definitions(xxhash PUBLIC XXH_NO_LONG_LONG=1)

FetchContent_Load(ekpyron xxhashct)
FetchContent_Populate(xxhashct)
]]

#[[
set(JSON_MultipleHeaders on)
set(JSON_DisableEnumSerialization on)
FetchContent_Load(nlohmann json)
FetchContent_MakeAvailable(json)
]]

#unable to use it now, because it change compiler flags and break the project
#FetchContent_Load(stephenberry glaze GIT_TAG main)
#FetchContent_MakeAvailable(glaze)

#FetchContent_Load(martinus robin-hood-hashing)
#FetchContent_MakeAvailable(robin-hood-hashing)

#FetchContent_Load(max0x7ba atomic_queue)
#FetchContent_Populate(atomic_queue)

FetchContent_Load(fmtlib fmt)
FetchContent_MakeAvailable(fmt)
target_compile_definitions(fmt PUBLIC FMT_USE_FULL_CACHE_DRAGONBOX=1)

#[[
set(SUBHOOK_STATIC on)
set(SUBHOOK_INSTALL off)
set(SUBHOOK_TESTS off)
FetchContent_Load(Zeex subhook)
FetchContent_MakeAvailable(subhook)
]]

#FetchContent_Load(m417z minhook GIT_TAG multihook)
FetchContent_Load(TsudaKageyu minhook)
FetchContent_MakeAvailable(minhook)

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
FetchContent_Load(gabime spdlog GIT_TAG v1.x)
FetchContent_MakeAvailable(spdlog)
target_compile_definitions(spdlog PUBLIC basic_runtime=runtime_format_string)
]]

#[[
set(SIGMATCH_DEV_MODE off)
set(SIGMATCH_BUILD_EXAMPLES off)
set(SIGMATCH_BUILD_BENCHMARKS off)
set(SIGMATCH_BUILD_TESTS off)
FetchContent_Load(SpriteOvO sigmatch GIT_TAG dev)
FetchContent_MakeAvailable(sigmatch)
]]

#FetchContent_Load(JustasMasiulis lazy_importer)
#FetchContent_Populate(lazy_importer)

# -------------------

add_subdirectory(${FD_IMPL_DIR}/imgui ${imgui_BINARY_DIR})
add_subdirectory(${FD_IMPL_DIR}/fd ${FD_OUT_DIR}/shared)
add_subdirectory(${FD_IMPL_DIR}/targets ${FD_OUT_DIR}/core)
#add_subdirectory(${FD_IMPL_DIR}/gui_test ${FD_OUT_DIR}/gui_test)
#add_subdirectory(${FD_IMPL_DIR}/dll ${FD_OUT_DIR}/dll)