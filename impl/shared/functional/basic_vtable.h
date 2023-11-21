#pragma once

#include <cstddef>

namespace fd
{
template <class T>
class basic_vtable final
{
    union
    {
        T* instance_;
        void*** vtable_;
    };

  public:
    basic_vtable(T* instance)
        : instance_(instance)
    {
    }

    T* instance() const
    {
        return instance_;
    }

    void* operator[](ptrdiff_t const function_index) const
    {
        return (*vtable_)[function_index];
    }
};
}