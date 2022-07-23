module;

#include <span>

export module fd.mem_block;
using DWORD = unsigned long;

class mem_block : public std::span<uint8_t>
{
    using _Base = std::span<uint8_t>;

  public:
    mem_block() = default;

    template <typename Bg>
    mem_block(Bg* const begin, const size_type mem_size = sizeof(uintptr_t))
        : _Base((uint8_t*)begin, mem_size)
    {
        static_assert(sizeof(Bg) == sizeof(uint8_t));
    }

    template <typename Bg, typename Ed>
    mem_block(Bg* const begin, Ed* const end)
        : _Base((uint8_t*)begin, (uint8_t*)end)
    {
        static_assert(sizeof(Bg) + sizeof(Ed) == sizeof(uint8_t) * 2);
    }

    explicit mem_block(const _Base span);

    /*template <class StorageType>
    mem_block find_block(const signature_known_bytes<StorageType>& rng) const
    {
        return find_block({rng.data( ),rng.size( )});
    }*/

    mem_block shift_to(pointer ptr) const;
    // mem_block subblock(size_t offset, size_t count = std::dynamic_extent) const;

    bool  have_flags(DWORD flags) const;
    bool  dont_have_flags(DWORD flags) const;
    DWORD get_flags() const;

    bool readable() const;
    bool readable_ex() const;
    bool writable() const;
    bool executable() const;
    bool code_padding() const;
};

export namespace fd
{
    using ::mem_block;
}
