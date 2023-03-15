include(FetchContent)
set(GIT_SHALLOW true)
set(FETCHCONTENT_QUIET off)
set(FETCHCONTENT_BASE_DIR ${FD_ROOT_DIR}/.deps1)
set(FETCHCONTENT_FULLY_DISCONNECTED on)

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

set(DISABLE_FORCE_DEBUG_POSTFIX on)
set(FT_DISABLE_ZLIB on)
set(FT_DISABLE_BZIP2 on)
set(FT_DISABLE_PNG on)
set(FT_DISABLE_HARFBUZZ on)
set(FT_DISABLE_BROTLI on)
fetchContent_Load(freetype freetype)
fetchContent_MakeAvailable(freetype)

fetchContent_Load(ocornut imgui GIT_TAG features/string_view)
fetchContent_Populate(imgui)

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

#unable to use it now, because it change compiler flags and break the project
#FetchContent_Load(stephenberry glaze GIT_TAG main)
#FetchContent_MakeAvailable(glaze)

#FetchContent_Load(martinus robin-hood-hashing)
#FetchContent_MakeAvailable(robin-hood-hashing)

#FetchContent_Load(max0x7ba atomic_queue)
#FetchContent_Populate(atomic_queue)

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

#set(SIGMATCH_DEV_MODE off)
#set(SIGMATCH_BUILD_EXAMPLES off)
#set(SIGMATCH_BUILD_BENCHMARKS off)
#set(SIGMATCH_BUILD_TESTS off)
#FetchContent_Load(SpriteOvO sigmatch GIT_TAG dev)
#FetchContent_MakeAvailable(sigmatch)

#FetchContent_Load(JustasMasiulis lazy_importer)
#FetchContent_Populate(lazy_importer)

# -------------------

add_subdirectory(${FD_PROJECTS_DIR}/imgui ${imgui_BINARY_DIR})
add_subdirectory(${FD_PROJECTS_DIR}/shared ${FD_OUT_DIR}/shared)
add_subdirectory(${FD_PROJECTS_DIR}/gui_test ${FD_OUT_DIR}/gui_test)
add_subdirectory(${FD_PROJECTS_DIR}/dll ${FD_OUT_DIR}/dll)