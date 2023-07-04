#pragma once

#include <cstdint>

namespace fd
{
struct native_data_map
{
    enum class field_type : int32_t
    {
        void_ = 0,  // No type or value
        floating,   // Any floating point value
        string,     // A string ID (return from ALLOC_STRING)
        vector,     // Any vector, QAngle, or AngularImpulse
        quaternion, // A quaternion
        integer,    // Any integer or enum
        boolean,    // boolean, implemented as an int, I may use this as a hint for compression
        short_,     // 2 byte integer
        character,  // a byte
        color32,    // 8-bit per channel r,g,b,a (32bit color)
        embedded,   // an embedded object with a datadesc, recursively traverse and embedded class/structure based on
                    // an additional typedescription
        custom,     // special type that contains function pointers to it's read/write/parse functions

        classptr, // CBaseEntity *
        ehandle,  // Entity handle
        edict,    // edict_t *

        position_vector, // A world coordinate (these are fixed up across level transitions automagically)
        time,            // a floating point time (these are fixed up automatically too!)
        tick,            // an integer tick count( fixed up similarly to time)
        modelname,       // Engine string that is a model name (needs precache)
        soundname,       // Engine string that is a sound name (needs precache)

        input,    // a list of inputed data fields (all derived from CMultiInputVar)
        function, // A class function pointer (Think, Use, etc)

        vmatrix, // a vmatrix (output coords are NOT worldspace)

        // note: Use float arrays for local transformations that don't need to be fixed up.
        vmatrix_worldspace,   // A VMatrix that maps some local space to world space (translation is fixed up on level
                              // transitions)
        matrix3x4_worldspace, // matrix3x4_t that maps some local space to world space (translation is fixed up on
                              // level transitions)

        interval,      // a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
        modelindex,    // a model index
        materialindex, // a material index (using the material precache string table)

        vector2d, // 2 floats
    };

    struct field
    {
        field_type type;
        char const *name;
        int offset;
        int offset_packed;
        int16_t size;
        int16_t flags;
        uint8_t pad_0014[12];
        native_data_map *description;
        char pad_0024[24];
    }; // Size: 0x003C

    field *fields;
    uint32_t fields_count;
    char const *name;
    native_data_map *base;

    bool chains_validated;
    // Have the "packed" offsets been computed
    bool packed_offsets_computed;
    int packed_size;
};
}