module;

#include <cstdint>

export module fd.mem_scanner;
export import fd.string;

using pointer = const uint8_t*;

class memory_range
{
  protected:
    pointer from_, to_;

  public:
    memory_range(pointer from, pointer to)
        : from_(from)
        , to_(to)
    {
    }

    memory_range(pointer from, const size_t size)
        : memory_range(from, from + size)
    {
    }
};

struct unknown_bytes_range;

class unknown_bytes_wrapped
{
    unknown_bytes_range* bytes_;

  public:
    unknown_bytes_wrapped(const fd::string_view sig);
    ~unknown_bytes_wrapped();

    size_t count() const;
    void* find_in(const pointer begin, const pointer end) const;
};

struct dummy_proj
{
    void* operator()(void* ptr) const
    {
        return ptr;
    }
};

template <typename To>
constexpr auto cast_helper_proj = [](void* ptr) {
    return reinterpret_cast<To>(ptr);
};

struct pattern_scanner : memory_range
{
    using memory_range::memory_range;

    void* operator()(const fd::string_view sig, const bool raw = false) const;
    void* operator()(const pointer begin, const size_t mem_size) const;

    template <typename Fn, typename Proj = dummy_proj>
    void operator()(const fd::string_view sig, const bool raw, Fn callback, Proj proj = {}) const
    {
        if (raw)
        {
            this->operator()((pointer)sig.data(), sig.size(), static_cast<Fn&&>(callback));
            return;
        }

        auto stop       = false;
        auto [from, to] = *this;

        const unknown_bytes_wrapped bytes(sig);
        const auto mem_size = bytes.count();

        do
        {
            const auto ptr = bytes.find_in(from, to);
            if (!ptr)
                break;
            callback(proj(ptr), mem_size, stop);
            if (stop)
                break;
            from += mem_size;
        }
        while (from <= to);
    }

    template <typename Fn, typename Proj = dummy_proj>
    void operator()(const pointer begin, const size_t mem_size, Fn callback, Proj proj = {}) const
    {
        auto stop        = false;
        auto finder      = *this;
        auto& [from, to] = finder;

        do
        {
            const auto ptr = finder(begin, mem_size);
            if (!ptr)
                break;
            callback(proj(ptr), mem_size, stop);
            if (stop)
                break;
            from += mem_size;
        }
        while (from <= to);
    }
};

struct xrefs_finder : memory_range
{
    using memory_range::memory_range;

    uintptr_t operator()(const uintptr_t addr) const;

    template <typename Fn>
    void operator()(const uintptr_t addr, Fn callback) const
    {
        const pattern_scanner scanner(from_, to_);
        scanner((pointer)&addr, sizeof(uintptr_t), callback, cast_helper_proj<uintptr_t>);
    }
};

export namespace fd
{
    using ::pattern_scanner;
    using ::xrefs_finder;
} // namespace fd
