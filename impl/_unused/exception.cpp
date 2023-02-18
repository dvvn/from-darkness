#include <fd/exception.h>

#include <utility>

namespace fd
{
static const unload_handler _DefaultUnloadHandler = [] {
    std::abort();
};

static auto _UnloadHandler = _DefaultUnloadHandler;

void unload()
{
    _UnloadHandler();
}

void set_unload(const unload_handler fn)
{
    _UnloadHandler = fn ? fn : _DefaultUnloadHandler;
}

void suspend()
{
    std::abort();
}

void unreachable()
{
#ifdef __cpp_lib_unreachable
    std::unreachable();
#else
    std::abort();
#endif
}
}