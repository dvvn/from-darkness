#pragma once

#include "core.h"

#include <fd/magic_cast.h>

namespace fd
{
class dos_header
{
    using reference = IMAGE_DOS_HEADER &;
    using pointer   = IMAGE_DOS_HEADER *;

    pointer ptr_;

  public:
    dos_header(pointer ptr)
        : ptr_(ptr)
    {
    }

    operator pointer() const
    {
        return ptr_;
    }

    pointer operator->() const
    {
        return ptr_;
    }

    reference operator*() const
    {
        return *ptr_;
    }
};

inline auto operator+(dos_header dos, size_t offset) -> magic_cast<void *, auto_cast_tag>
{
    union
    {
        void *ptr;
        uintptr_t addr;
    };

    ptr = dos;
    addr += offset;
    return ptr;
}
} // namespace fd