cmake_minimum_required(VERSION 3.14)

set(BUILD_SHARED_LIBS off) 
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) 

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED on)

set(_PROJ_NAME fd)
string(TOUPPER ${_PROJ_NAME} _PROJ_NAME_UPPER)

project(${_PROJ_NAME} CXX)

set(_PROJ_IMPL_DIR ${PROJECT_SOURCE_DIR}/impl)
set(_PROJ_WORK_DIR ${_PROJ_IMPL_DIR}/${_PROJ_NAME})

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "")
    set(CMAKE_BUILD_TYPE ${CMAKE_CONFIGURATION_TYPES})
endif()

include(FetchContent)
set(GIT_SHALLOW true)
set(FETCHCONTENT_QUIET off)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/.deps1)

set(_GIT_DIR "C:/git" CACHE STRING "The repos storage")

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_DEBUG_POSTFIX _d)
  set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
endif()

include(ProcessorCount)
ProcessorCount(_CPUS)
message(DEBUG "Cpu's count: ${_CPUS}")

if(MSVC)
  # Whole program optimization
  if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(/GL)
    add_link_options(/LTCG)
    add_compile_options(/Qpar /Qvec-report:1 /Qpar-report:1)
  endif()  
  add_compile_options(/Zc:preprocessor /Zc:__cplusplus /permissive-)
  add_compile_options(/wd5103 /wd4291 /wd4250)
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
list(APPEND _PROJ_LIBS )

set(DISABLE_FORCE_DEBUG_POSTFIX on)
set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
FetchContent_Load(freetype freetype)
FetchContent_MakeAvailable(freetype)

FetchContent_Load(ocornut imgui)
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
FetchContent_Load(gabime spdlog GIT_TAG v1.x)
FetchContent_MakeAvailable(spdlog)
target_compile_definitions(spdlog PRIVATE basic_runtime=runtime_format_string)
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

file(GLOB _IMGUI_FILES ${imgui_SOURCE_DIR}/*.cpp)
list(APPEND _IMGUI_FILES ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp)
if(MSVC)
  list(APPEND _IMGUI_FILES ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp)
else()

endif()
add_library(imgui ${_IMGUI_FILES})
target_link_libraries(imgui PRIVATE freetype)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_compile_definitions(imgui PUBLIC
    IMGUI_USER_CONFIG="${_PROJ_WORK_DIR}/gui/config.h"
)

#-------

file(GLOB_RECURSE _PROJ_FILES ${_PROJ_WORK_DIR}/*.cpp)
add_library(impl ${_PROJ_FILES})
target_link_libraries(impl PUBLIC 
    #function2
    robin_hood::robin_hood   
    nlohmann_json
    subhook
    fmt
    imgui
    xxhash
    spdlog
    sigmatch
    Boost::thread
    Boost::algorithm
    Boost::container
    Boost::ptr_container
    Boost::container_hash
    Boost::json
    Boost::sort
    Boost::io
    Boost::dll
    Boost::process
    Boost::geometry
    Boost::gil
    Boost::uuid
    Boost::fusion
    Boost::hana
)
target_include_directories(impl PUBLIC
    ${_PROJ_IMPL_DIR}
    ${xxhct_SOURCE_DIR}
    #${veque_SOURCE_DIR}/include
    #${atomic_queue_SOURCE_DIR}/include
    ${lazy_importer_SOURCE_DIR}/include
)
target_compile_definitions(impl PUBLIC
    ${_PROJ_NAME_UPPER}_WORK_DIR=${_PROJ_WORK_DIR}
    ${_PROJ_NAME_UPPER}_ROOT_DIR=${CMAKE_CURRENT_LIST_DIR}
    #[[ RMLUI_STATIC_LIB RMLUI_DIR=${rml_SOURCE_DIR} ]]
)

file(GLOB_RECURSE _GUI_TEST_FILES ${_PROJ_IMPL_DIR}/gui_test/*.cpp ${_PROJ_IMPL_DIR}/gui_test/*.ixx)
add_executable(gui_test ${_GUI_TEST_FILES})
target_link_libraries(gui_test impl d3d9.lib)
target_compile_definitions(gui_test PRIVATE ${_PROJ_NAME_UPPER}_GUI_TEST)

file(GLOB_RECURSE _LIBRARY_FILES ${_PROJ_IMPL_DIR}/library/*.cpp ${_PROJ_IMPL_DIR}/library/*.ixx)
add_library(library SHARED ${_LIBRARY_FILES})
target_link_libraries(library impl)
target_compile_definitions(library PRIVATE ${_PROJ_NAME_UPPER}_LIBRARY)
