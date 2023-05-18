project(targets CXX)

function(prepare _name)
	add_library(${_name} ${_name}.cpp)
	target_link_libraries(${_name} PUBLIC fd)
endfunction()

add_library(cheat SHARED main.cpp)
target_link_libraries(cheat PUBLIC fd)
target_compile_definitions(cheat PUBLIC FD_SHARED_LIB)

add_executable(gui main.cpp own_backend.cpp)
target_link_libraries(gui PUBLIC fd)