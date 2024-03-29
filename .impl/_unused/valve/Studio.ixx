module;

#include <cstdint>

export module fd.Studio;
export import fd.math.vector4;
export import fd.math.matrix3x4;

export namespace fd::csgo
{
    using RadianEuler = float[3];

    enum bone_flags
    {
        BONE_CALCULATE_MASK        = 0x1F,
        BONE_PHYSICALLY_SIMULATED  = 0x01, // bone is physically simulated when physics are active
        BONE_PHYSICS_PROCEDURAL    = 0x02, // procedural when physics is active
        BONE_ALWAYS_PROCEDURAL     = 0x04, // bone is always procedurally animated
        BONE_SCREEN_ALIGN_SPHERE   = 0x08, // bone aligns to the screen, not constrained in motion.
        BONE_SCREEN_ALIGN_CYLINDER = 0x10, // bone aligns to the screen, constrained by it's own axis.

        BONE_USED_MASK           = 0x0007FF00,
        BONE_USED_BY_ANYTHING    = 0x0007FF00,
        BONE_USED_BY_HITBOX      = 0x00000100, // bone (or child) is used by a hit box
        BONE_USED_BY_ATTACHMENT  = 0x00000200, // bone (or child) is used by an attachment point
        BONE_USED_BY_VERTEX_MASK = 0x0003FC00,
        BONE_USED_BY_VERTEX_LOD0 = 0x00000400, // bone (or child) is used by the toplevel model via skinned vertex
        BONE_USED_BY_VERTEX_LOD1 = 0x00000800,
        BONE_USED_BY_VERTEX_LOD2 = 0x00001000,
        BONE_USED_BY_VERTEX_LOD3 = 0x00002000,
        BONE_USED_BY_VERTEX_LOD4 = 0x00004000,
        BONE_USED_BY_VERTEX_LOD5 = 0x00008000,
        BONE_USED_BY_VERTEX_LOD6 = 0x00010000,
        BONE_USED_BY_VERTEX_LOD7 = 0x00020000,
        BONE_USED_BY_BONE_MERGE  = 0x00040000, // bone is available for bone merge to occur against it

        // BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
        // BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )
    };

    inline constexpr auto MAX_NUM_LODS   = 8;
    inline constexpr auto MAXSTUDIOBONES = /*128*/ 256; // total bones actually used

    enum bone_types
    {

        BONE_TYPE_MASK       = 0x00F00000,
        BONE_FIXED_ALIGNMENT = 0x00100000, // bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

        BONE_HAS_SAVEFRAME_POS   = 0x00200000, // Vector48
        BONE_HAS_SAVEFRAME_ROT64 = 0x00400000, // Quaternion64
        BONE_HAS_SAVEFRAME_ROT32 = 0x00800000, // Quaternion32
    };

    enum hitgroups
    {

        HITGROUP_GENERIC = 0,
        HITGROUP_HEAD,
        HITGROUP_CHEST,
        HITGROUP_STOMACH,
        HITGROUP_LEFTARM,
        HITGROUP_RIGHTARM,
        HITGROUP_LEFTLEG,
        HITGROUP_RIGHTLEG,
        HITGROUP_GEAR = 10
    };

    enum modtype_t
    {
        mod_bad = 0,
        mod_brush,
        mod_sprite,
        mod_studio
    };

    enum Hitboxes
    {
        HITBOX_HEAD,
        HITBOX_NECK,
        HITBOX_PELVIS,
        HITBOX_STOMACH,
        HITBOX_LOWER_CHEST,
        HITBOX_CHEST,
        HITBOX_UPPER_CHEST,
        HITBOX_RIGHT_THIGH,
        HITBOX_LEFT_THIGH,
        HITBOX_RIGHT_CALF,
        HITBOX_LEFT_CALF,
        HITBOX_RIGHT_FOOT,
        HITBOX_LEFT_FOOT,
        HITBOX_RIGHT_HAND,
        HITBOX_LEFT_HAND,
        HITBOX_RIGHT_UPPER_ARM,
        HITBOX_RIGHT_FOREARM,
        HITBOX_LEFT_UPPER_ARM,
        HITBOX_LEFT_FOREARM,
        HITBOX_MAX
    };

    using MDLHandle_t = unsigned short;

    struct mstudiobone_t
    {
        int sznameindex;

        char* const pszName() const
        {
            return ((char*)this) + sznameindex;
        }

        int parent;
        int bonecontroller[6]; // bone controller index, -1 == none
        math::vector3 pos;
        math::quaternion quat;
        RadianEuler rot;
        // compression scale
        math::vector3 posscale;
        math::vector3 rotscale;

        math::matrix3x4 poseToBone;
        math::quaternion qAlignment;
        int flags;
        int proctype;
        int procindex;
        mutable int physicsbone;

        void* pProcedure() const
        {
            if (procindex == 0)
                return 0;
            else
                return (std::uint8_t*)this + procindex;
        };

        int surfacepropidx;

        char* const pszSurfaceProp() const
        {
            return ((char*)this) + surfacepropidx;
        }

        int GetSurfaceProp() const
        {
            return surfacepropLookup;
        }

        int contents;
        int surfacepropLookup;
        int Pad01[7];

      private:
        // No copy constructors allowed
        mstudiobone_t(const mstudiobone_t& vOther);
    };

    struct mstudiobbox_t
    {
        int bone;
        int group;
        math::vector3 bbmin;
        math::vector3 bbmax;
        int szhitboxnameindex;
        int32_t Pad01[3];
        float Radius;
        int32_t Pad02[4];

        const char* GetName()
        {
            if (!szhitboxnameindex)
                return nullptr;
            return (const char*)((uint8_t*)this + szhitboxnameindex);
        }
    };

    struct mstudiohitboxset_t
    {
        int sznameindex;
        int numhitboxes;
        int hitboxindex;

        const char* GetName()
        {
            if (!sznameindex)
                return nullptr;
            return (const char*)((uint8_t*)this + sznameindex);
        }

        mstudiobbox_t* GetHitbox(int i)
        {
            if (i > numhitboxes)
                return nullptr;
            return (mstudiobbox_t*)((uint8_t*)this + hitboxindex) + i;
        }
    };

    struct model_t
    {
        void* fnHandle;        // 0x0000
        char szName[260];      // 0x0004
        __int32 nLoadFlags;    // 0x0108
        __int32 nServerCount;  // 0x010C
        __int32 type;          // 0x0110
        __int32 flags;         // 0x0114
        math::vector3 vecMins; // 0x0118
        math::vector3 vecMaxs; // 0x0124
        float radius;          // 0x0130
        char pad[0x1C];        // 0x0134
    };                         // Size=0x0150

    class studiohdr_t
    {
      public:
        __int32 id;                  // 0x0000
        __int32 version;             // 0x0004
        long checksum;               // 0x0008
        char szName[64];             // 0x000C
        __int32 length;              // 0x004C
        math::vector3 vecEyePos;     // 0x0050
        math::vector3 vecIllumPos;   // 0x005C
        math::vector3 vecHullMin;    // 0x0068
        math::vector3 vecHullMax;    // 0x0074
        math::vector3 vecBBMin;      // 0x0080
        math::vector3 vecBBMax;      // 0x008C
        __int32 flags;               // 0x0098
        __int32 numbones;            // 0x009C
        __int32 boneindex;           // 0x00A0
        __int32 numbonecontrollers;  // 0x00A4
        __int32 bonecontrollerindex; // 0x00A8
        __int32 numhitboxsets;       // 0x00AC
        __int32 hitboxsetindex;      // 0x00B0
        __int32 numlocalanim;        // 0x00B4
        __int32 localanimindex;      // 0x00B8
        __int32 numlocalseq;         // 0x00BC
        __int32 localseqindex;       // 0x00C0
        __int32 activitylistversion; // 0x00C4
        __int32 eventsindexed;       // 0x00C8
        __int32 numtextures;         // 0x00CC
        __int32 textureindex;        // 0x00D0

        mstudiohitboxset_t* GetHitboxSet(int i)
        {
            if (i > numhitboxsets)
                return nullptr;
            return (mstudiohitboxset_t*)((uint8_t*)this + hitboxsetindex) + i;
        }

        mstudiobone_t* GetBone(int i)
        {
            if (i > numbones)
                return nullptr;
            return (mstudiobone_t*)((uint8_t*)this + boneindex) + i;
        }
    }; // Size=0x00D4
} // namespace fd::csgo
