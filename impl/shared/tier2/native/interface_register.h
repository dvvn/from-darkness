#pragma once
#include "tier1/string/view.h"
#include "tier2/core.h"

#undef interface

namespace FD_TIER2(native)
{
class interface_register
{
    void* (*create_)();
    char const* name_;
    interface_register* next_;

    interface_register() = default;

  public:
    ~interface_register() = delete;

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

struct basic_interface
{
  protected:
    basic_interface() = default;
    basic_interface(interface_register::iterator current, string_view name, void** ptr);
};

template <class T>
struct interface : protected basic_interface
{
    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = T const*;
    using reference       = T&;
    using const_reference = T const&;

  private:
    pointer ptr_;

  public:
    interface(pointer ptr)
        : ptr_{ptr}
    {
    }

    interface(interface_register::iterator current, string_view name)
        : basic_interface{current, name, reinterpret_cast<void**>(&ptr_)}
    {
    }

    const_pointer operator->() const
    {
        return ptr_;
    }

    const_reference operator*() const
    {
        return *ptr_;
    }
};
} // namespace FD_TIER2(native)
