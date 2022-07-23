module;

#include <algorithm>
#include <ranges>
#include <span>

module fd.mem_block;

// import fd.mem_protect;

mem_block::mem_block(const _Base span)
    : _Base(span)
{
}

mem_block mem_block::shift_to(pointer ptr) const
{
    const auto offset = std::distance(this->data(), ptr);
    const auto tmp    = this->subspan(offset);
    return { tmp.data(), tmp.size() };
}
