#pragma once

#include <cstdint>

namespace fd
{
struct mem_protect
{
#ifdef _WIN32
    using size_type = unsigned long;
#else
#pragma error not implemented
#endif

  private:
    void*     addr_;
    size_t    size_;
    size_type oldFlags_;

    mem_protect(mem_protect const& other);
    mem_protect& operator=(mem_protect const&);

  public:
    ~mem_protect();

    mem_protect();
    mem_protect(void* addr, size_t size, size_type newFlags);

    mem_protect(mem_protect&& other) noexcept;
    mem_protect& operator=(mem_protect&& other) noexcept;

    bool restore();
    bool has_value() const;
};
} // namespace fd
