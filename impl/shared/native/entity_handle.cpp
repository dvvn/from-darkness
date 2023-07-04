#include "entity_handle.h"

#include <cassert>

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

namespace fd
{
uint32_t native_entity_handle::index() const
{
    assert(value != INVALID_EHANDLE_INDEX);
    return value & ENT_ENTRY_MASK;
}
}