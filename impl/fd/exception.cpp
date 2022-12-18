#include <fd/exception.h>
#include <fd/functional.h>

#include <atomic>

using namespace fd;

static unload_handler _DefaultUnloadHandler = [] {
    std::abort();
};

static std::atomic _UnloadHandler(_DefaultUnloadHandler);

void fd::unload()
{
    invoke(_UnloadHandler.load(std::memory_order_relaxed));
}

void fd::set_unload(const unload_handler fn)
{
    _UnloadHandler.store(fn != nullptr ? fn : _DefaultUnloadHandler, std::memory_order_relaxed);
}

void fd::suspend()
{
    std::abort();
}

void fd::unreachable()
{
#ifdef __cpp_lib_unreachable
    std::unreachable();
#else
    std::abort();
#endif
}