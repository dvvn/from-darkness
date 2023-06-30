#pragma once

// ReSharper disable CppInconsistentNaming
struct IDirect3DDevice9;

namespace fd
{
struct native_render_backend
{
    using element_type = IDirect3DDevice9;
    using pointer      = element_type *;

  private:
    pointer ptr_;

  public:
    native_render_backend()
    {
        (void)this;
    }

    native_render_backend(pointer ptr)
        : ptr_(ptr)
    {
    }

    template <typename T>
    native_render_backend(T &src) requires requires { static_cast<pointer>(src); }
        : ptr_(src)
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
};

}