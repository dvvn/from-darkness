module;

#include <fd/functional.h>

#include <Windows.h>

#include <optional>
#include <stdexcept>

module fd.mem_info;

using namespace fd;

static std::optional<MEMORY_BASIC_INFORMATION> _Update(void* address)
{
    constexpr size_t class_size = sizeof(MEMORY_BASIC_INFORMATION);

    MEMORY_BASIC_INFORMATION info;
    if (VirtualQuery(address, &info, class_size) == class_size)
        return info;

    return {};
}

static_assert(sizeof(mem_info) == sizeof(size_t) + sizeof(size_type) * 2);

mem_info::mem_info(void* address)
{
    if (!update(address))
        throw std::runtime_error("Unable to update the memory info!");
}

mem_info::mem_info() = default;

bool mem_info::update(void* address)
{
    const auto info = _Update(address);
    if (info)
    {
        size  = info->RegionSize;
        state = info->State;
        flags = info->Protect;
    }
    return info.has_value();
}

constexpr auto _Memory_info_looper = [](void* address, size_t limit, auto&& fn) {
    for (;;)
    {
        auto info = _Update(address);
        if (!info)
            return false;
        if (info->State != MEM_COMMIT)
            return false;
        const auto& my_info = reinterpret_cast<mem_info&>(info->RegionSize);
        if (!invoke(fn, my_info))
            return false;

        auto& pos = reinterpret_cast<uint8_t*&>(address);

        const auto start = static_cast<uint8_t*>(info->BaseAddress);
        const auto end   = start + my_info.size;

        const size_t dist = std::distance(pos, end);
        if (dist >= limit)
            return true;

        pos = end;
        limit -= dist;
    }
};

//-----

static auto _Have_flags(const size_type flags)
{
    return bind_back(_Memory_info_looper, [flags](const mem_info& info) {
        return !!(info.flags & flags);
    });
}

static auto _Dont_have_flags(const size_type flags)
{
    return bind_back(_Memory_info_looper, [flags](const mem_info& info) {
        return !(info.flags & flags);
    });
}

constexpr size_type _Page_read_flags    = PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE;
constexpr size_type _Page_write_flags   = PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOMBINE;
constexpr size_type _Page_execute_flags = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

bool mem_executable(void* address, const size_t size)
{
    return invoke(_Have_flags(_Page_execute_flags), address, size);
}

bool mem_have_flags(void* address, const size_t size, const size_type flags)
{
    return invoke(_Have_flags(flags), address, size);
}
