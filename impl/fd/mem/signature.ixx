module;

#include <cstdint>

export module fd.signature;
export import fd.string;

using pointer = const uint8_t*;

struct signature_finder
{
    pointer from, to;

    signature_finder(pointer from, pointer to)
        : from(from)
        , to(to)
    {
    }

    signature_finder(pointer from, const size_t size)
        : from(from)
        , to(from + size)
    {
    }

    void* operator()(const fd::string_view sig, const bool raw = false) const;
    void* operator()(const pointer begin, const size_t mem_size) const;
};

export namespace fd
{
    using ::signature_finder;
} // namespace fd
