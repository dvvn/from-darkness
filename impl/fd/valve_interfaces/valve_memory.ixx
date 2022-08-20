export module fd.valve.memory;

template <class T, class N = int>
class memory
{
    T* memory_;
    int allocation_count_;
    int grow_size_;

  public:
    T& operator[](N i)
    {
        return memory_[i];
    }

    const T& operator[](N i) const
    {
        return memory_[i];
    }

    T* data()
    {
        return memory_;
    }

    const T* data() const
    {
        return memory_;
    }

    int capacity() const
    {
        return allocation_count_;
    }

    bool is_externally_allocated() const
    {
        return grow_size_ < 0;
    }

    //--
};

export namespace fd::valve
{
    using ::memory;
}
