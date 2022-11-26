#include <fd/mem_protect.h>

#include <Windows.h>

#include <memory>
#include <optional>

using namespace fd;

static bool _Set_flags(LPVOID addr, const size_t size, const DWORD new_flags, DWORD& old_flags)
{
    return VirtualProtect(addr, size, new_flags, &old_flags);
}

static bool _Set_flags(LPVOID addr, const size_t size, const DWORD new_flags)
{
    DWORD unused;
    return _Set_flags(addr, size, new_flags, unused);
}

mem_protect::~mem_protect()
{
    if (has_value())
        _Set_flags(addr_, size_, old_flags_);
}

mem_protect::mem_protect()
    : old_flags_(0)
{
}

mem_protect::mem_protect(void* addr, const size_t size, const size_type new_flags)
{
    if (_Set_flags(addr, size, new_flags, old_flags_))
    {
        addr_ = addr;
        size_ = size;
    }
}

mem_protect::mem_protect(const mem_protect& other)            = default;
mem_protect& mem_protect::operator=(const mem_protect& other) = default;

mem_protect::mem_protect(mem_protect&& other)
{
    *this            = other;
    other.old_flags_ = 0;
}

mem_protect& mem_protect::operator=(mem_protect&& other)
{
    auto old = *this;
    *this    = other;
    other    = old;
    return *this;
}

bool mem_protect::restore()
{
    if (!has_value())
        return false;
    if (!_Set_flags(addr_, size_, old_flags_))
        return false;
    old_flags_ = 0;
    return true;
}

bool mem_protect::has_value() const
{
    return old_flags_ != 0;
}
