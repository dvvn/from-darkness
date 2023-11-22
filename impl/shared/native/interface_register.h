#pragma once
#include <cstddef>

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

    void* try_get() const;

    class iterator
    {
        interface_register const* current_;

      public:
        iterator();
        iterator(interface_register const* current);

        iterator& operator++(int);
        iterator operator++() const;

        interface_register const& operator*() const;
        interface_register const* operator->() const;

        explicit operator bool() const;
        bool operator==(iterator const& other) const;
    };

    friend iterator find(interface_register const* current, char const* name, size_t name_length, bool name_contains_version);
};

interface_register::iterator find(interface_register const* current, char const* name, size_t name_length);
interface_register::iterator find(interface_register const* current, char const* name, size_t name_length, bool name_contains_version);

interface_register::iterator begin(interface_register const* reg);
interface_register::iterator end(interface_register const* reg);
} // namespace fd::native
