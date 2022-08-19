module;

#include <cstdint>

#include <Windows.h>

module fd.mem_info;
import fd.functional.invoke;

mem_info::mem_info(void* address)
{
    update(address);
}

mem_info::mem_info()
{
    valid_ = false;
}

void mem_info::update(void* address)
{
    constexpr size_t class_size = sizeof(MEMORY_BASIC_INFORMATION);

    MEMORY_BASIC_INFORMATION info;
    if (!(valid_ = VirtualQuery(address, &info, class_size) == class_size))
        return;

    size  = info.RegionSize;
    state = info.State;
    flags = info.Protect;
}

mem_info::operator bool() const
{
    return valid_;
}

template <typename Fn>
static bool _Memory_info_looper(mem_info_iterator itr, size_t limit, const Fn fn)
{
    do
    {
        // memory isn't commit!
        if (itr->state != MEM_COMMIT)
            break;
        if (!fd::invoke(fn, *itr))
            break;
        // found good result
        if (itr->size >= limit)
            return true;
        limit -= itr->size;
    }
    while (++itr);

    return false;
}

//-----

mem_info_iterator::mem_info_iterator(void* address)
    : address_(address)
{
    info_.update(address);
}

mem_info_iterator& mem_info_iterator::operator++()
{
    address_ = static_cast<uint8_t*>(address_) + info_.size;
    info_.update(address_);
    return *this;
}

mem_info_iterator mem_info_iterator::operator++(int)
{
    auto copy = *this;
    operator++();
    return copy;
}

mem_info& mem_info_iterator::operator*()
{
    return info_;
}

const mem_info& mem_info_iterator::operator*() const
{
    return info_;
}

mem_info* mem_info_iterator::operator->()
{
    return &info_;
}

const mem_info* mem_info_iterator::operator->() const
{
    return &info_;
}

mem_info_iterator::operator bool() const
{
    return static_cast<bool>(info_);
}

//-----

static bool _Have_flags(void* const mem, const size_t size, const size_t flags)
{
    return _Memory_info_looper(mem, size, [flags](const mem_info& info) {
        return !!(info.flags & flags);
    });
}

static bool _Dont_have_flags(void* const mem, const size_t size, const size_t flags)
{
    return _Memory_info_looper(mem, size, [flags](const mem_info& info) {
        return !(info.flags & flags);
    });
}

constexpr size_t _Page_read_flags    = PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE;
constexpr size_t _Page_write_flags   = PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOMBINE;
constexpr size_t _Page_execute_flags = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

bool mem_executable(void* address, const size_t size)
{
    return _Have_flags(address, size, _Page_execute_flags);
}

bool mem_have_flags(void* address, const size_t size, const size_t flags)
{
    return _Have_flags(address, size, flags);
}
