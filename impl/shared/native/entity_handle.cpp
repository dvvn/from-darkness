#include "entity_handle.h"

#include <cassert>

// How many bits to use to encode an edict.
constexpr auto max_edict_bits = 11; // # of bits needed to represent max edicts;

// Max # of edicts in a level
constexpr auto max_edicts = (1 << max_edict_bits);

// Used for networking ehandles.
constexpr auto num_ent_entry_bits        = (max_edict_bits + 2);
constexpr auto num_ent_entries           = (1 << num_ent_entry_bits);
constexpr auto invalid_ehandle_index     = 0XFFFFFFFF;

constexpr auto num_serial_num_bits       = 16; // 32 - num_ent_entry_bits;
constexpr auto num_serial_num_shift_bits = (32 - num_serial_num_bits);
constexpr auto ent_entry_mask            = ((1 << num_serial_num_bits) - 1);

namespace fd
{
auto native_entity_handle::index() const -> size_type
{
    assert(value != invalid_ehandle_index);
    return value & ent_entry_mask;
}
}