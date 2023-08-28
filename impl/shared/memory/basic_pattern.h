#pragma once

#include <cstdint>

namespace fd
{
namespace detail
{
#ifdef _DEBUG
using pattern_size_type       = size_t;
using pattern_difference_type = ptrdiff_t;
#else
using pattern_size_type       = uint8_t;
using pattern_difference_type = int8_t;
#endif
} // namespace detail

struct basic_pattern_segment
{
    using size_type       = detail::pattern_size_type;
    using difference_type = detail::pattern_difference_type;

    using value_type = uint8_t;
    using pointer    = uint8_t const*;

  protected:
    ~basic_pattern_segment() = default;

  public:
    virtual pointer begin() const = 0;
    virtual pointer end() const   = 0;

    virtual size_type tail() const = 0;
};

struct basic_pattern
{
    using size_type       = detail::pattern_size_type;
    using difference_type = detail::pattern_difference_type;

    using iterator = basic_pattern_segment const* const*;

  protected:
    ~basic_pattern() = default;

  public:
    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;

    // segments count
    virtual size_type segments() const = 0;
    // segemnts length in bytes
    // virtual size_t length() const     = 0;
    // virtual size_t abs_length() const = 0;
};
} // namespace fd