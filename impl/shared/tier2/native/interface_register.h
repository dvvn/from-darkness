#pragma once
#include "tier1/string/view.h"
#include "tier2/core.h"

namespace FD_TIER2(native)
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

        iterator operator++(int);
        iterator& operator++();

        interface_register const& operator*() const;
        interface_register const* operator->() const;

        explicit operator bool() const;
        bool operator==(iterator const& other) const;
    };

    iterator find(string_view name) const;
    iterator find(string_view name, bool name_contains_version) const;
};

interface_register::iterator begin(interface_register const& reg);
interface_register::iterator end(interface_register const& reg);

void* get(interface_register const& reg, string_view name);
} // namespace FD_TIER2(native)
