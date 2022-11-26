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
        void* addr_;
        size_t size_;
        size_type old_flags_;

        mem_protect(const mem_protect& other);
        mem_protect& operator=(const mem_protect&);

      public:
        ~mem_protect();

        mem_protect();
        mem_protect(void* addr, const size_t size, const size_type new_flags);

        mem_protect(mem_protect&& other);
        mem_protect& operator=(mem_protect&& other);

        bool restore();
        bool has_value() const;
    };
} // namespace fd
