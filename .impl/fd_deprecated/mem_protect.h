#pragma once

#include <boost/core/noncopyable.hpp>

#include <cstddef>

namespace fd
{
struct mem_protect : boost::noncopyable
{
#ifdef _WIN32
    using size_type = unsigned long;
#else
#pragma error not implemented
#endif

  private:
    void *addr_;
    size_t size_;
    size_type old_flags_;

  public:
    ~mem_protect();

    mem_protect();
    mem_protect(void *addr, size_t size, size_type new_flags);

    mem_protect(mem_protect &&other) noexcept;
    mem_protect &operator=(mem_protect &&other) noexcept;

    bool restore();
    bool has_value() const;
};
} // namespace fd