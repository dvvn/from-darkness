#include "entity_handle.h"

#include <cassert>
#include <execution>

// How many bits to use to encode an edict.
constexpr auto MAX_EDICT_BITS = 11; // # of bits needed to represent max edicts
// Max # of edicts in a level
constexpr auto MAX_EDICTS     = 1 << MAX_EDICT_BITS;

// Used for networking ehandles.
constexpr auto NUM_ENT_ENTRY_BITS    = MAX_EDICT_BITS + 2;
constexpr auto NUM_ENT_ENTRIES       = 1 << NUM_ENT_ENTRY_BITS;
constexpr auto INVALID_EHANDLE_INDEX = 0xFFFFFFFF;

constexpr auto NUM_SERIAL_NUM_BITS       = 16 /*32 - NUM_ENT_ENTRY_BITS*/;
constexpr auto NUM_SERIAL_NUM_SHIFT_BITS = 32 - NUM_SERIAL_NUM_BITS;
constexpr auto ENT_ENTRY_MASK            = (1 << NUM_SERIAL_NUM_BITS) - 1;

namespace fd::valve
{
uint32_t get_entity_index(entity_handle handle)
{
    auto index = handle.value;
    assert(index != INVALID_EHANDLE_INDEX);
    return index & ENT_ENTRY_MASK;
}
}