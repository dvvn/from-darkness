project(fd_dll CXX)

add_library(fd_dll SHARED main.cpp)
target_link_libraries(fd_dll fd_shared)
