#pragma once

#include <cstdint>

namespace fd
{
struct basic_pattern_segment
{
    friend struct basic_pattern;

    using value_type = uint8_t;
    using pointer    = value_type const *;
    using size_type  = uint8_t;

    virtual pointer begin() const = 0;
    virtual pointer end() const   = 0;
    virtual size_type gap() const = 0;

  protected:
    // class size for iterator
    virtual size_t size() const = 0;
};

struct basic_pattern
{
    using segment = basic_pattern_segment const *;

    struct iterator
    {
        union
        {
            void *current;
            segment target;
        };

        iterator(segment current)
            : target(current)
        {
        }

        iterator &operator++()
        {
            current = static_cast<uint8_t *>(current) + target->size();
            return *this;
        }

        bool operator==(iterator const &other) const
        {
            return target == other.target;
        }
    };

    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;
};
} // namespace fd

namespace std
{
inline size_t size(fd::basic_pattern_segment const &i);
}