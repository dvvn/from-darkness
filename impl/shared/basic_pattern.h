#pragma once

#include <cstdint>

namespace fd
{
struct basic_pattern_segment
{
    using value_type = uint8_t;
    using pointer    = value_type const *;
    using size_type  = uint8_t;

  protected:
    ~basic_pattern_segment() = default;

  public:
    virtual size_type self_size() const = 0;

    virtual pointer begin() const = 0;
    virtual pointer end() const   = 0;

    virtual size_type tail() const = 0;
};

struct basic_pattern
{
    using segment = basic_pattern_segment const *;

    struct iterator
    {
        using pointer   = basic_pattern_segment const *;
        using reference = basic_pattern_segment const &;

      private:
        union
        {
            uint8_t *current_;
            pointer target_;
        };

      public:
        iterator(segment current)
            : target_(current)
        {
        }

        iterator &operator++()
        {
            current_ += target_->self_size();
            return *this;
        }

        iterator operator+() const
        {
            auto copy = *this;
            return ++copy;
        }

        pointer operator->() const
        {
            return target_;
        }

        reference operator*() const
        {
            return *target_;
        }

        bool operator==(iterator const &other) const
        {
            return target_ == other.target_;
        }
    };

  protected:
    ~basic_pattern() = default;

  public:
    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;

    // segments count
    virtual size_t segments() const = 0;
    // segemnts length in bytes
    // virtual size_t length() const     = 0;
    // virtual size_t abs_length() const = 0;
};
} // namespace fd