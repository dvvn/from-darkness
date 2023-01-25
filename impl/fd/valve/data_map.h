#pragma once

#include <span>

namespace fd
{
namespace valve
{

struct data_map;

enum data_map_description_type : int32_t
{
    FIELD_VOID = 0,   // No type or value
    FIELD_FLOAT,      // Any floating point value
    FIELD_STRING,     // A string ID (return from ALLOC_STRING)
    FIELD_VECTOR,     // Any vector, QAngle, or AngularImpulse
    FIELD_QUATERNION, // A quaternion
    FIELD_INTEGER,    // Any integer or enum
    FIELD_BOOLEAN,    // boolean, implemented as an int, I may use this as a hint for compression
    FIELD_SHORT,      // 2 byte integer
    FIELD_CHARACTER,  // a byte
    FIELD_COLOR32,    // 8-bit per channel r,g,b,a (32bit color)
    FIELD_EMBEDDED,   // an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
    FIELD_CUSTOM,     // special type that contains function pointers to it's read/write/parse functions

    FIELD_CLASSPTR, // CBaseEntity *
    FIELD_EHANDLE,  // Entity handle
    FIELD_EDICT,    // edict_t *

    FIELD_POSITION_VECTOR, // A world coordinate (these are fixed up across level transitions automagically)
    FIELD_TIME,            // a floating point time (these are fixed up automatically too!)
    FIELD_TICK,            // an integer tick count( fixed up similarly to time)
    FIELD_MODELNAME,       // Engine string that is a model name (needs precache)
    FIELD_SOUNDNAME,       // Engine string that is a sound name (needs precache)

    FIELD_INPUT,    // a list of inputed data fields (all derived from CMultiInputVar)
    FIELD_FUNCTION, // A class function pointer (Think, Use, etc)

    FIELD_VMATRIX, // a vmatrix (output coords are NOT worldspace)

    // NOTE: Use float arrays for local transformations that don't need to be fixed up.
    FIELD_VMATRIX_WORLDSPACE,   // A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
    FIELD_MATRIX3X4_WORLDSPACE, // matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

    FIELD_INTERVAL,      // a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
    FIELD_MODELINDEX,    // a model index
    FIELD_MATERIALINDEX, // a material index (using the material precache string table)

    FIELD_VECTOR2D, // 2 floats
};

struct data_map_description
{
    data_map_description_type type;
    const char*               name;
    int                       offset;
    int                       offset_packed;
    int16_t                   size;
    int16_t                   flags;
    uint8_t                   pad_0014[12];
    data_map*                 description;
    char                      pad_0024[24];
}; // Size: 0x003C

struct data_map
{
    std::span<data_map_description> data;
    const char*                     name;
    data_map*                       base;

    bool chains_validated;
    // Have the "packed" offsets been computed
    bool packed_offsets_computed;
    int  packed_size;
};

} // namespace valve

template <typename>
class range_view;

template <>
class range_view<const valve::data_map*>
{
    using ptr_type = const valve::data_map*;

    struct iterator
    {
        using reference = const valve::data_map&;

      private:
        ptr_type ptr_;

      public:
        iterator(const ptr_type ptr = nullptr)
            : ptr_(ptr)
        {
        }

        reference operator*() const
        {
            return *ptr_;
        }

        iterator& operator++()
        {
            ptr_ = ptr_->base;
            return *this;
        }

        iterator operator++(int)
        {
            auto ret = *this;
            ptr_     = ptr_->base;
            return ret;
        }

        bool operator==(const iterator&) const = default;
    };

    ptr_type begin_;

  public:
    range_view(const ptr_type begin)
        : begin_(begin)
    {
    }

    iterator begin() const
    {
        return begin_;
    }

    iterator end() const
    {
        (void)this;
        return nullptr;
    }
};

template <std::convertible_to<const valve::data_map*> P>
range_view(P) -> range_view<const valve::data_map*>;

};