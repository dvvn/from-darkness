#include <fd/assert.h>
#include <fd/valve/base_handle.h>
#include <fd/valve/client_entity.h>
#include <fd/valve/client_entity_list.h>

using namespace fd;
using namespace valve;

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

base_handle::base_handle()
{
    m_Index = INVALID_EHANDLE_INDEX;
}

base_handle::base_handle(const base_handle& other)
{
    m_Index = other.m_Index;
}

base_handle::base_handle(handle_entity* pHandleObj)
{
    if (!pHandleObj)
    {
        m_Index = INVALID_EHANDLE_INDEX;
    }
    else
    {
        *this = pHandleObj->GetRefEHandle();
    }
}

base_handle::base_handle(int iEntry, int iSerialNumber)
{
    FD_ASSERT(iEntry >= 0 && (iEntry & ENT_ENTRY_MASK) == iEntry);
    FD_ASSERT(iSerialNumber >= 0 && iSerialNumber < (1 << NUM_SERIAL_NUM_BITS));
    m_Index = iEntry | (iSerialNumber << /*NUM_ENT_ENTRY_BITS*/ NUM_SERIAL_NUM_SHIFT_BITS);
}

// Even if this returns true, Get() still can return return a non-null value.
// This just tells if the handle has been initted with any values.
bool base_handle::IsValid() const
{
    return m_Index != INVALID_EHANDLE_INDEX;
}

int32_t base_handle::GetEntryIndex() const
{
    if (!IsValid())
        return NUM_ENT_ENTRIES - 1;
    return m_Index & ENT_ENTRY_MASK;
}

int32_t base_handle::GetSerialNumber() const
{
    return m_Index >> /*NUM_ENT_ENTRY_BITS*/ NUM_SERIAL_NUM_SHIFT_BITS;
}

int32_t base_handle::ToInt() const
{
    return (int32_t)m_Index;
}

namespace fd::valve
{
    extern client_entity_list* entity_list;
}

handle_entity* base_handle::Get() const
{
    /*if (!IsValid( ))
        return 0;*/
    return entity_list->GetClientEntityFromHandle(*this);
    // FD_ASSERT_PANIC("Not implemented");
}
