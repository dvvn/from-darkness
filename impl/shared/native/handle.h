#pragma once

#include <cassert>

namespace fd::native
{
inline constexpr auto INVALID_EHANDLE_INDEX     = 0xffffffff;
inline constexpr auto ENT_ENTRY_MASK            = 0x7fff;
inline constexpr auto NUM_SERIAL_NUM_SHIFT_BITS = 15;

class CBaseHandle
{
    uint32_t index_;

  public:
    CBaseHandle(uint32_t const value = INVALID_EHANDLE_INDEX)
        : index_{value}
    {
    }

    /*CBaseHandle(int entry, int serialNumber)
        : index_{entry | (serialNumber << NUM_SERIAL_NUM_SHIFT_BITS)}
    {
    }*/

    explicit operator bool() const
    {
        return index_ != INVALID_EHANDLE_INDEX;
    }

    uint32_t entry_index() const
    {
        // return IsValid() ? index_ & ENT_ENTRY_MASK : ENT_ENTRY_MASK;
        assert(operator bool());
        return index_ & ENT_ENTRY_MASK;
    }

    uint32_t serial_number() const
    {
        return index_ >> NUM_SERIAL_NUM_SHIFT_BITS;
    }

    /*int ToInt() const
    {
        return static_cast<int>(index_);
    }*/

    bool operator==(CBaseHandle const other) const
    {
        return index_ == other.index_;
    }

    /*C_BaseEntity* Get() const
    {
        if (!IsValid())
            return nullptr;

        C_BaseEntity* ent = CGameEntitySystem::GetBaseEntity(GetEntryIndex());
        if (!ent || ent->GetRefEHandle() != *this)
            return nullptr;

        return ent;
    }*/
};

// template <typename T>
// class CHandle : public CBaseHandle
//{
//   public:
//     auto Get() const
//     {
//         return reinterpret_cast<T*>(Get());
//     }
// };
} // namespace fd::native