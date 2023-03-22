#pragma once

namespace fd::valve
{
template <typename T>
class vector
{
    // memory class
    T *memory_;
    int capacity_;
    int grow_size_;

    int size_;
    T *debug_elements_;

  public:
    vector() = delete;

    T &operator[](int i)
    {
        return memory_[i];
    }

    T const &operator[](int i) const
    {
        return memory_[i];
    }

    T *data()
    {
        return memory_;
    }

    T const *data() const
    {
        return memory_;
    }

    int size() const
    {
        return size_;
    }

    int capacity() const
    {
        return capacity_;
    }

    bool is_externally_allocated() const
    {
        return grow_size_ < 0;
    }
};
} // namespace fd::valve
