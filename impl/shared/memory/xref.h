#pragma once


#include <cstdint>

namespace fd
{
template <bool Owned>
struct xref;

template <>
struct xref<false> final
{
    using pointer  = uintptr_t const*;
    using iterator = uint8_t const*;

  private:
    pointer value_;

  public:
    xref(uintptr_t&&) = delete;

    xref(uintptr_t const& value)
        : value_(&value)
    {
    }

    [[deprecated]]
    pointer get() const
    {
        return value_;
    }

    iterator begin() const
    {
        return reinterpret_cast<iterator>(value_);
    }

    iterator end() const
    {
        return reinterpret_cast<iterator>(value_ + 1);
    }
};

template <>
struct xref<true> final
{
    using pointer  = uintptr_t const*;
    using iterator = uint8_t const*;

  private:
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

    [[deprecated]]
    pointer get() const&
    {
        return &value_;
    }

    iterator begin() const&
    {
        return reinterpret_cast<iterator>(&value_);
    }

    iterator end() const&
    {
        return reinterpret_cast<iterator>(&value_ + 1);
    }
};

template <typename T>
xref(T&&) -> xref<std::is_rvalue_reference_v<T&&>>;
} // namespace fd