module;

#include <cstdint>

export module fd.mem_info;

class mem_info
{
    bool valid_;

  public:
    mem_info(void* address);
    mem_info();

    void update(void* address);
    explicit operator bool() const;

    size_t size;
    size_t state;
    size_t flags;
};

class mem_info_iterator
{
    void* address_;
    mem_info info_;

  public:
    mem_info_iterator(void* address);

    mem_info_iterator& operator++();
    mem_info_iterator operator++(int);

    mem_info& operator*();
    const mem_info& operator*() const;

    mem_info* operator->();
    const mem_info* operator->() const;

    explicit operator bool() const;
};

bool mem_executable(void* address, const size_t size);
bool mem_have_flags(void* address, const size_t size, const size_t flags);

export namespace fd
{
    using ::mem_info;
    using ::mem_info_iterator;

    using ::mem_executable;
    using ::mem_have_flags;
} // namespace fd
