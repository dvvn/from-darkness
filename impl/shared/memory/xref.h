#pragma once

#include <cstdint>

namespace fd
{
template <bool Owned>
class xref;

template <>
class xref<false> final
{
    uintptr_t const* value;

  public:
    xref(uintptr_t&&) = delete;

    xref(uintptr_t const& value)
        : value(&value)
    {
    }

    uintptr_t const* get() const
    {
        return value;
    }
};

template <>
class xref<true> final
{
    uintptr_t value_;

  public:
    xref(uintptr_t&& value)
        : value_(value)
    {
    }

    xref(void* value)
        : value_(reinterpret_cast<uintptr_t>(value))
    {
    }

    xref(uintptr_t const& value) = delete;

    uintptr_t const* get() const&
    {
        return &value_;
    }
};

template <typename T>
xref(T&&) -> xref<std::is_rvalue_reference_v<T&&>>;
} // namespace fd