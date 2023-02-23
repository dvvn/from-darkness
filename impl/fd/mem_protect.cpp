#include "mem_protect.h"

#include <Windows.h>

namespace fd
{
static_assert(sizeof(SIZE_T) == sizeof(size_t));

static bool _set_flags(const LPVOID addr, const SIZE_T size, const DWORD newFlags, DWORD& oldFlags)
{
    return VirtualProtect(addr, size, newFlags, &oldFlags);
}

static bool _set_flags(const LPVOID addr, const SIZE_T size, const DWORD newFlags)
{
    DWORD unused;
    return _set_flags(addr, size, newFlags, unused);
}

mem_protect::~mem_protect()
{
    if (has_value())
        _set_flags(addr_, size_, oldFlags_);
}

mem_protect::mem_protect()
    : addr_(nullptr)
    , size_(0)
    , oldFlags_(0)
{
}

mem_protect::mem_protect(void* addr, const size_t size, const size_type newFlags)
{
    if (_set_flags(addr, size, newFlags, oldFlags_))
    {
        addr_ = addr;
        size_ = size;
    }
}

mem_protect::mem_protect(mem_protect const& other)            = default;
mem_protect& mem_protect::operator=(mem_protect const& other) = default;

mem_protect::mem_protect(mem_protect&& other) noexcept
{
    *this           = other;
    other.oldFlags_ = 0;
}

mem_protect& mem_protect::operator=(mem_protect&& other) noexcept
{
    auto const old = *this;
    *this          = other;
    other          = old;
    return *this;
}

bool mem_protect::restore()
{
    if (has_value() && _set_flags(addr_, size_, oldFlags_))
    {
        oldFlags_ = 0;
        return true;
    }
    return false;
}

bool mem_protect::has_value() const
{
    return oldFlags_ != 0;
}
} // namespace fd