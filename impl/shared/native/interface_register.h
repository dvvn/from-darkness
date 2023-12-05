#pragma once

#undef interface

namespace fd::native
{
class interface_register
{
    void* (*create_)();
    char const* name_;
    interface_register* next_;

  public:
    ~interface_register() = delete;
    interface_register()  = default;

    void* get() const
    {
        return create_();
    }

    char const* name() const
    {
        return name_;
    }

    interface_register* next() const
    {
        return next_;
    }
};
} // namespace fd::native
