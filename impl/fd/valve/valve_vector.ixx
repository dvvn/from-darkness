
export module fd.valve.vector;
export import fd.valve.memory;

template <class T, class A = fd::valve::memory<T>>
class vector
{
  public:
    using allocator_type = A;
    using value_type     = T;
    using size_type      = int;

    vector(const vector& other)            = delete;
    vector(vector&& other)                 = delete;
    vector& operator=(const vector& other) = delete;
    vector& operator=(vector&& other)      = delete;

    vector() = delete;

    T* begin()
    {
        return memory_.data();
    }

    T* end()
    {
        return memory_.data() + size_;
    }

    const T* begin() const
    {
        return memory_.data();
    }

    const T* end() const
    {
        return memory_.data() + size_;
    }

    T& operator[](const size_type i)
    {
        return memory_[i];
    }

    const T& operator[](const size_type i) const
    {
        return memory_[i];
    }

    T* data()
    {
        return memory_.data();
    }

    const T* data() const
    {
        return memory_.data();
    }

    size_type size() const
    {
        return size_;
    }

  private:
    allocator_type memory_;
    size_type size_;
    T* debug_elements_;
};

export namespace fd::valve
{
    using ::vector;
}
