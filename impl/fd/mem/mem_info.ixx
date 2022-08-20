module;

#include <cstdint>

export module fd.mem_info;

#ifdef _WIN32
using size_type = unsigned long;
#else
#pragma error not implemented
#endif

struct mem_info
{
    mem_info(void* address);
    mem_info();

    bool update(void* address);

    size_t size;
    size_type state;
    size_type flags;
};

bool mem_executable(void* address, const size_t size);
bool mem_have_flags(void* address, const size_t size, const size_type flags);

export namespace fd
{
    using ::mem_info;

    using ::mem_executable;
    using ::mem_have_flags;
} // namespace fd
