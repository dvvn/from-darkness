project(fd CXX)

file(GLOB_RECURSE _FILES ${FD_DIR}/*.cpp)
add_library(fd ${_FILES})

list(TRANSFORM BOOST_INCLUDE_LIBRARIES PREPEND Boost::)
target_link_libraries(fd PUBLIC 
    #function2
    #robin_hood::robin_hood
    #nlohmann_json::nlohmann_json
    #glaze::glaze
    #subhook::subhook
    minhook
    fmt::fmt
    imgui::imgui
    #xxHash::xxhash
    #spdlog::spdlog
    #sigmatch::sigmatch
    #Boost::lexical_cast
    Boost::unordered
    Boost::nowide
    Boost::lambda2
    ${BOOST_INCLUDE_LIBRARIES}
)
target_include_directories(fd PUBLIC
    ${FD_IMPL_DIR}
    ${xxhct_SOURCE_DIR}
    #${veque_SOURCE_DIR}/include
    #${atomic_queue_SOURCE_DIR}/include
    ${lazy_importer_SOURCE_DIR}/include
    ${x86RetSpoof_SOURCE_DIR}
)
target_compile_definitions(fd PUBLIC
    FD_WORK_DIR=${FD_DIR}
    FD_ROOT_DIR=${FD_ROOT_DIR}
    FD_IMPL_DIR=${FD_IMPL_DIR}
    FD_VALVE_DIR=${FD_DIR}/valve
    FD_DEFAULT_LOG_LEVEL=${FD_DEFAULT_LOG_LEVEL}
)