#include <fd/valve/entity_handle.h>

#include <cassert>

namespace fd::valve
{
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

handle::handle()
    : index_(INVALID_EHANDLE_INDEX)
{
}

handle::handle(handle const &other) = default;

handle::handle(entity_handle *handle_obj)
    : index_(handle_obj ? handle_obj->get().index_ : INVALID_EHANDLE_INDEX)
{
}

handle::handle(int index, int serial_number)
{
    assert(index >= 0 && (index & ENT_ENTRY_MASK) == index);
    assert(serial_number >= 0 && serial_number < (1 << NUM_SERIAL_NUM_BITS));
    index_ = index | (serial_number << /*NUM_ENT_ENTRY_BITS*/ NUM_SERIAL_NUM_SHIFT_BITS);
}

// Even if this returns true, Get() still can return return a non-null value.
// This just tells if the handle has been initted with any values.
handle::operator bool() const
{
    return index_ != INVALID_EHANDLE_INDEX;
}

int32_t handle::entry_index() const
{
    assert(index_ != INVALID_EHANDLE_INDEX);
    return index_ & ENT_ENTRY_MASK;
}

int32_t handle::serial_number() const
{
    return index_ >> /*NUM_ENT_ENTRY_BITS*/ NUM_SERIAL_NUM_SHIFT_BITS;
}

int32_t handle::to_int() const
{
    return static_cast<int32_t>(index_);
}

#if 0
extern client_entity_list *entity_list;

entity_handle *handle::Get() const
{
    /*if (!IsValid( ))
        return 0;*/
    return entity_list->GetClientEntityFromHandle(*this);
    // FD_ASSERT_PANIC("Not implemented");
}
#endif
}