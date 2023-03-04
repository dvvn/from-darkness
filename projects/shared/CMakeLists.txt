project(fd_shared CXX)

file(GLOB_RECURSE _FILES ${FD_DIR}/*.cpp)
add_library(fd_shared ${_FILES})
target_link_libraries(fd_shared PUBLIC 
    #function2
    #robin_hood::robin_hood
    nlohmann_json::nlohmann_json
    glaze::glaze
    subhook::subhook
    fmt::fmt
    imgui::imgui
    xxHash::xxhash
    spdlog::spdlog
    #sigmatch::sigmatch
    #Boost::thread
    Boost::algorithm
    Boost::container
    Boost::container_hash
    Boost::json
    Boost::sort
    Boost::io
    #Boost::dll
    Boost::process
    Boost::geometry
    #Boost::gil
    #Boost::uuid
    #Boost::fusion
    Boost::hana
    Boost::static_string
    #Boost::lexical_cast
)
target_include_directories(fd_shared PUBLIC
    ${FD_IMPL_DIR}
    ${xxhct_SOURCE_DIR}
    #${veque_SOURCE_DIR}/include
    #${atomic_queue_SOURCE_DIR}/include
    ${lazy_importer_SOURCE_DIR}/include
)
target_compile_definitions(fd_shared PUBLIC
    FD_WORK_DIR=${FD_DIR}
    FD_ROOT_DIR=${FD_ROOT_DIR}
)