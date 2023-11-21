#pragma once

namespace fd::native
{
class interface_register
{
    void* (*create_)();
    char const* name_;
    interface_register* next_;

  public:
    interface_register() = delete;

    void* get() const;
    char const* name() const;
    interface_register* next() const;
};

interface_register* find_unique(interface_register* first, interface_register* last, char const* name, size_t name_length);
interface_register* find(interface_register* first, interface_register* last, char const* name, size_t name_length);
} // namespace fd::native
