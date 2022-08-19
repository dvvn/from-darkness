module;

#include <limits.h>
#include <optional>

export module fd.mem_protect;

struct mem_protect
{
#ifdef _WIN32
    using size_type = uint32_t;
#else
#pragma error not implemented
#endif

  private:
    struct data
    {
        void* addr;
        size_t size;
        size_t flags;

        size_t set() const;
    };

    std::optional<data> info_;

  public:
    mem_protect();
    mem_protect(const void* addr, const size_t size, const size_type new_flags);
    mem_protect(mem_protect&& other);
    ~mem_protect();

    mem_protect(const mem_protect&)            = delete;
    mem_protect& operator=(const mem_protect&) = delete;
    mem_protect& operator=(mem_protect&& other);

    bool restore();
    bool has_value() const;
};

export namespace fd
{
    using ::mem_protect;
}
