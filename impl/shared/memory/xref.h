#pragma once

#include "basic_xref.h"

namespace fd
{
template <bool Owned>
class xref;

template <>
class xref<false> final : public basic_xref
{
    uintptr_t const *value;

  public:
    xref(uintptr_t &&) = delete;

    xref(uintptr_t const &value)
        : value(&value)
    {
    }

    pointer get() const override
    {
        return value;
    }
};

template <>
class xref<true> final: public basic_xref
{
    uintptr_t value_;

  public:
    xref(uintptr_t &&value)
        : value_(value)
    {
    }

    xref(void *value)
        : value_(reinterpret_cast<uintptr_t>(value))
    {
    }

    xref(uintptr_t const &value) = delete;

    pointer get() const override
    {
        return &value_;
    }
};

template <typename T>
xref(T &&) -> xref<std::is_rvalue_reference_v<T &&>>;

} // namespace fd