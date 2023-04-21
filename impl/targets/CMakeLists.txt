project(targets CXX)

function(prepare _name)
	add_library(${_name} ${_name}.cpp)
	target_link_libraries(${_name} PUBLIC fd)
endfunction()

prepare(logger)
prepare(dummy_logger)
prepare(console)

add_library(cheat SHARED main.cpp)
if(CMAKE_BUILD_TYPE MATCHES Debug)
	target_link_libraries(cheat logger console)
else()
	target_link_libraries(cheat dummy_logger)
endif()

add_executable(gui main.cpp logger own_backend.cpp)
target_link_libraries(gui logger)
