#include <fd/exception.h>
#include <fd/functional.h>

#include <atomic>

using namespace fd;

static unload_handler _DefaultUnloadHandler = [] {
    std::abort();
};

static std::atomic _UnloadHandler(_DefaultUnloadHandler);

namespace fd
{
    void unload()
    {
        invoke(_UnloadHandler.load(std::memory_order_relaxed));
    }

    void set_unload(const unload_handler fn)
    {
        _UnloadHandler.store(fn != nullptr ? fn : _DefaultUnloadHandler, std::memory_order_relaxed);
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