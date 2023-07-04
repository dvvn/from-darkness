#include "mem_protect.h"

#include <Windows.h>

#include <utility>

namespace fd
{

static bool set_flags(LPVOID addr, SIZE_T size, DWORD new_flags, DWORD &old_flags)
{
    return VirtualProtect(addr, size, new_flags, &old_flags);
}

static bool set_flags(LPVOID addr, SIZE_T size, DWORD new_flags)
{
    DWORD unused;
    return set_flags(addr, size, new_flags, unused);
}

mem_protect::~mem_protect()
{
    if (has_value())
        set_flags(addr_, size_, old_flags_);
}

mem_protect::mem_protect()
    : addr_(nullptr)
    , size_(0)
    , old_flags_(0)
{
}

mem_protect::mem_protect(void *addr, size_t size, size_type new_flags)
{
    if (set_flags(addr, size, new_flags, old_flags_))
    {
        addr_ = addr;
        size_ = size;
    }
}

mem_protect::mem_protect(mem_protect &&other) noexcept
    : addr_(other.addr_)
    , size_(other.size_)
    , old_flags_(std::exchange(other.old_flags_, 0))
{
}

mem_protect &mem_protect::operator=(mem_protect &&other) noexcept
{
    using std::swap;
    swap(addr_, other.addr_);
    swap(size_, other.size_);
    swap(old_flags_, other.old_flags_);
    return *this;
}

bool mem_protect::restore()
{
    if (has_value() && set_flags(addr_, size_, old_flags_))
    {
        old_flags_ = 0;
        return true;
    }
    return false;
}

bool mem_protect::has_value() const
{
    static_assert(sizeof(SIZE_T) == sizeof(size_t));
    return old_flags_ != 0;
}
} // namespace fd