#pragma once

#include <fd/hidden_ptr.h>

using IMAGE_DOS_HEADER = struct _IMAGE_DOS_HEADER;

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

hidden_ptr operator+(dos_header dos, std::integral auto offset)
{
    return hidden_ptr(dos) + offset;
}
} // namespace fd