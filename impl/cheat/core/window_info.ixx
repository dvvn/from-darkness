module;

#include <utility>

export module cheat.tools.window_info;

export namespace cheat::tools
{
    std::pair<size_t, size_t> window_size( ) noexcept;//width, heigh
    std::pair<void*, long> window_proc( ) noexcept;//current, default
}