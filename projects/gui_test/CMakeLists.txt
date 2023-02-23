project(fd_gui_test CXX)

add_executable(fd_gui_test main.cpp backend.cpp)
target_link_libraries(fd_gui_test fd_shared d3d9.lib)
#target_compile_definitions(fd_gui_test PRIVATE FD_GUI_TEST)