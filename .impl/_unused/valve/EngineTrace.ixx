module;

#include <cstdint>
#include <tuple>

export module fd.EngineTrace;
export import fd.client_class;
export import fd.math.vector4;
export import fd.math.matrix3x4;

export namespace fd::csgo
{
    enum DispSurfFlags : uint16_t
    {
        DISPSURF_FLAG_SURFACE   = (1 << 0),
        DISPSURF_FLAG_WALKABLE  = (1 << 1),
        DISPSURF_FLAG_BUILDABLE = (1 << 2),
        DISPSURF_FLAG_SURFPROP1 = (1 << 3),
        DISPSURF_FLAG_SURFPROP2 = (1 << 4)
    };

    enum BspFlags : uint32_t
    {
        CONTENTS_EMPTY = 0,

        CONTENTS_SOLID        = 0x1,
        CONTENTS_WINDOW       = 0x2,
        CONTENTS_AUX          = 0x4,
        CONTENTS_GRATE        = 0x8,
        CONTENTS_SLIME        = 0x10,
        CONTENTS_WATER        = 0x20,
        CONTENTS_BLOCKLOS     = 0x40,
        CONTENTS_OPAQUE       = 0x80,
        LAST_VISIBLE_CONTENTS = CONTENTS_OPAQUE,

        ALL_VISIBLE_CONTENTS = (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS - 1)),

        CONTENTS_TESTFOGVOLUME        = 0x100,
        CONTENTS_UNUSED               = 0x200,
        CONTENTS_BLOCKLIGHT           = 0x400,
        CONTENTS_TEAM1                = 0x800,
        CONTENTS_TEAM2                = 0x1000,
        CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
        CONTENTS_MOVEABLE             = 0x4000,
        CONTENTS_AREAPORTAL           = 0x8000,
        CONTENTS_PLAYERCLIP           = 0x10000,
        CONTENTS_MONSTERCLIP          = 0x20000,
        CONTENTS_CURRENT_0            = 0x40000,
        CONTENTS_CURRENT_90           = 0x80000,
        CONTENTS_CURRENT_180          = 0x100000,
        CONTENTS_CURRENT_270          = 0x200000,
        CONTENTS_CURRENT_UP           = 0x400000,
        CONTENTS_CURRENT_DOWN         = 0x800000,

        CONTENTS_ORIGIN = 0x1000000,

        CONTENTS_MONSTER     = 0x2000000,
        CONTENTS_DEBRIS      = 0x4000000,
        CONTENTS_DETAIL      = 0x8000000,
        CONTENTS_TRANSLUCENT = 0x10000000,
        CONTENTS_LADDER      = 0x20000000,
        CONTENTS_HITBOX      = 0x40000000,
    };

    enum SurfaceFlags : uint16_t
    {
        SURF_LIGHT    = 0x0001,
        SURF_SKY2D    = 0x0002,
        SURF_SKY      = 0x0004,
        SURF_WARP     = 0x0008,
        SURF_TRANS    = 0x0010,
        SURF_NOPORTAL = 0x0020,
        SURF_TRIGGER  = 0x0040,
        SURF_NODRAW   = 0x0080,

        SURF_HINT = 0x0100,

        SURF_SKIP      = 0x0200,
        SURF_NOLIGHT   = 0x0400,
        SURF_BUMPLIGHT = 0x0800,
        SURF_NOSHADOWS = 0x1000,
        SURF_NODECALS  = 0x2000,
        SURF_NOPAINT   = SURF_NODECALS,
        SURF_NOCHOP    = 0x4000,
        SURF_HITBOX    = 0x8000,
    };

    enum ContentMasks : uint32_t
    {
        // -----------------------------------------------------
        // spatial content masks - used for spatial queries (traceline,etc.)
        // -----------------------------------------------------
        MASK_ALL                   = (0xFFFFFFFF),
        MASK_SOLID                 = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE),
        MASK_PLAYERSOLID           = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE),
        MASK_NPCSOLID              = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE),
        MASK_NPCFLUID              = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER),
        MASK_WATER                 = (CONTENTS_WATER | CONTENTS_MOVEABLE | CONTENTS_SLIME),
        MASK_OPAQUE                = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE),
        MASK_OPAQUE_AND_NPCS       = (MASK_OPAQUE | CONTENTS_MONSTER),
        MASK_BLOCKLOS              = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_BLOCKLOS),
        MASK_BLOCKLOS_AND_NPCS     = (MASK_BLOCKLOS | CONTENTS_MONSTER),
        MASK_VISIBLE               = (MASK_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE),
        MASK_VISIBLE_AND_NPCS      = (MASK_OPAQUE_AND_NPCS | CONTENTS_IGNORE_NODRAW_OPAQUE),
        MASK_SHOT                  = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_HITBOX),
        MASK_SHOT_BRUSHONLY        = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_DEBRIS),
        MASK_SHOT_HULL             = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE),
        MASK_SHOT_PORTAL           = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER),
        MASK_SOLID_BRUSHONLY       = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE),
        MASK_PLAYERSOLID_BRUSHONLY = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_PLAYERCLIP | CONTENTS_GRATE),
        MASK_NPCSOLID_BRUSHONLY    = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE),
        MASK_NPCWORLDSTATIC        = (CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE),
        MASK_NPCWORLDSTATIC_FLUID  = (CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP),
        MASK_SPLITAREAPORTAL       = (CONTENTS_WATER | CONTENTS_SLIME),
        MASK_CURRENT               = (CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN),
        MASK_DEADSOLID             = (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_GRATE),
    };

    class client_entity;
    class Ray_t;
    class CGameTrace;
    typedef CGameTrace trace_t;
    class collideable;
    class ITraceListData;
    class CPhysCollide;
    struct virtualmeshlist_t;
    class handle_entity;

    enum class TraceType
    {
        TRACE_EVERYTHING = 0,
        TRACE_WORLD_ONLY,
        TRACE_ENTITIES_ONLY,
        TRACE_EVERYTHING_FILTER_PROPS,
    };

    class ITraceFilter
    {
      public:
        virtual bool ShouldHitEntity(handle_entity* pEntity, int contentsMask) = 0;
        virtual TraceType GetTraceType() const                                 = 0;
    };

#if 0

	// This is the one most normal traces will inherit from
	class CTraceFilter : public ITraceFilter
	{
	public:
		auto ShouldHitEntity( handle_entity* pEntityHandle, int /*contentsMask*/) -> bool
		{
			return !(pEntityHandle == pSkip);
		}

		virtual auto GetTraceType( ) const -> TraceType
		{
			return TraceType::TRACE_EVERYTHING;
		}

		void* pSkip;
	};

	class CTraceFilterSkipEntity : public ITraceFilter
	{
	public:
		CTraceFilterSkipEntity( handle_entity* pEntityHandle)
		{
			pSkip = pEntityHandle;
		}

		auto ShouldHitEntity( handle_entity* pEntityHandle, int /*contentsMask*/) -> bool
		{
			return !(pEntityHandle == pSkip);
		}

		virtual auto GetTraceType( ) const -> TraceType
		{
			return TraceType::TRACE_EVERYTHING;
		}

		void* pSkip;
	};

	class CTraceFilterEntitiesOnly : public ITraceFilter
	{
	public:
		auto ShouldHitEntity( handle_entity* pEntityHandle, int /*contentsMask*/) -> bool
		{
			return true;
		}

		virtual auto GetTraceType( ) const -> TraceType
		{
			return TraceType::TRACE_ENTITIES_ONLY;
		}
	};

	//-----------------------------------------------------------------------------
	// Classes need not inherit from these
	//-----------------------------------------------------------------------------

	class CTraceFilterPlayersOnlySkipOne : public ITraceFilter
	{
	public:
		CTraceFilterPlayersOnlySkipOne(client_entity* ent)
		{
			pEnt = ent;
		}

		auto ShouldHitEntity( handle_entity* pEntityHandle, int /*contentsMask*/) -> bool
		{
			return pEntityHandle != pEnt && static_cast<client_entity*>(pEntityHandle)->GetClientClass( )->ClassID == ClassId::CCSPlayer;
		}

		virtual auto GetTraceType( ) const -> TraceType
		{
			return TraceType::TRACE_ENTITIES_ONLY;
		}

	private:
		client_entity* pEnt;
	};

	class CTraceFilterSkipTwoEntities : public ITraceFilter
	{
	public:
		CTraceFilterSkipTwoEntities(client_entity* ent1, client_entity* ent2)
		{
			pEnt1 = ent1;
			pEnt2 = ent2;
		}

		auto ShouldHitEntity( handle_entity* pEntityHandle, int /*contentsMask*/) -> bool
		{
			return !(pEntityHandle == pEnt1 || pEntityHandle == pEnt2);
		}

		virtual auto GetTraceType( ) const -> TraceType
		{
			return TraceType::TRACE_EVERYTHING;
		}

	private:
		client_entity* pEnt1;
		client_entity* pEnt2;
	};

#endif

    template <bool Ignore, TraceType T, class... E>
    class CTraceFilter : public ITraceFilter
    {
      public:
        CTraceFilter(E*... skip)
            : skip_{ skip... }
        {
            static_assert((std::derived_from<E, handle_entity> && ...));
            static_assert(T != TraceType::TRACE_WORLD_ONLY);
        }

        bool ShouldHitEntity(handle_entity* ent, int /*contentsMask*/) override
        {
            const auto found = this->find(ent, std::make_index_sequence<sizeof...(E)>());
            if constexpr (Ignore)
                return found;
            else
                return !found;
        }

        TraceType GetTraceType() const final
        {
            return T;
        }

      protected:
        CTraceFilter* CTraceFilterPtr()
        {
            return this;
        }

      private:
        template <size_t... I>
        bool find(handle_entity* ent, std::index_sequence<I...>) const
        {
            return ((std::get<I>(skip_) == ent) || ...);
        }

        std::tuple<E*...> skip_;
    };

    template <bool Ignore, class... E>
    class CTraceFilterPlayers final : public CTraceFilter<Ignore, TraceType::TRACE_ENTITIES_ONLY, E...>
    {
      public:
        CTraceFilterPlayers(E*... skip)
            : CTraceFilter(skip...)
        {
        }

        bool ShouldHitEntity(handle_entity* ent, int contents_mask) override
        {
            auto client_ent = static_cast<client_entity*>(ent);
            if (client_ent->GetClientClass()->ClassID != ClassId::CCSPlayer)
                return false;

            return this->CTraceFilterPtr()->ShouldHitEntity(ent, contents_mask);
        }
    };

    class CTraceFilterWorldOnly : public ITraceFilter
    {
      public:
        bool ShouldHitEntity(handle_entity*, int) override;
        TraceType GetTraceType() const override;
    };

    class CTraceFilterWorldAndPropsOnly : public ITraceFilter
    {
      public:
        bool ShouldHitEntity(handle_entity*, int) override;
        TraceType GetTraceType() const override;
    };

    class CTraceFilterHitAll final : public ITraceFilter
    {
      public:
        bool ShouldHitEntity(handle_entity*, int) override;
        TraceType GetTraceType() const override;
    };

    enum class DebugTraceCounterBehavior_t
    {
        kTRACE_COUNTER_SET = 0,
        kTRACE_COUNTER_INC,
    };

    //-----------------------------------------------------------------------------
    // Enumeration interface for EnumerateLinkEntities
    //-----------------------------------------------------------------------------
    class IEntityEnumerator
    {
      public:
        // This gets called with each handle
        virtual bool EnumEntity(handle_entity* pHandleEntity) = 0;
    };

    struct BrushSideInfo_t
    {
        math::vector4 plane;  // The plane of the brush side
        unsigned short bevel; // Bevel plane?
        unsigned short thin;  // Thin?
    };

    class CPhysCollide;

    struct vcollide_t
    {
        unsigned short solidCount : 15;
        unsigned short isPacked   : 1;
        unsigned short descSize;
        // VPhysicsSolids
        CPhysCollide** solids;
        char* pKeyValues;
        void* user_data;
    };

    struct cmodel_t
    {
        math::vector3 mins, maxs;
        math::vector3 origin; // for sounds or lights
        int headnode;
        vcollide_t vcollisionData;
    };

    struct csurface_t
    {
        const char* name;
        short surfaceProps;
        unsigned short flags; // BUGBUG: These are declared per surface, not per material, but this database is per-material now
    };

    struct cplane_t
    {
        math::vector3 normal;
        float dist;
        uint8_t type;     // for fast side tests
        uint8_t signbits; // signx + (signy<<1) + (signz<<1)
        uint8_t pad[2];
    };

    //-----------------------------------------------------------------------------
    // A ray...
    //-----------------------------------------------------------------------------
    class Ray_t
    {
      public:
        math::vector3_aligned m_Start;       // starting point, centered within the extents
        math::vector3_aligned m_Delta;       // direction + length of the ray
        math::vector3_aligned m_StartOffset; // Add this to m_Start to Get the actual ray start
        math::vector3_aligned m_Extents;     // Describes an axis aligned box extruded along a ray
        const math::matrix3x4* WorldAxisTransform = nullptr;
        bool m_IsRay;   // are the extents zero?
        bool m_IsSwept; // is delta != 0?

        Ray_t() = default;

        void Init(const math::vector3& start, const math::vector3& end);
        void Init(const math::vector3& start, const math::vector3& end, const math::vector3& mins, const math::vector3& maxs);

        // math::vector3 InvDelta( ) const;
    };

    class CBaseTrace
    {
      public:
        CBaseTrace() = default;

        bool IsDispSurface();
        bool IsDispSurfaceWalkable();
        bool IsDispSurfaceBuildable();
        bool IsDispSurfaceProp1();
        bool IsDispSurfaceProp2();

        // these members are aligned!!
        math::vector3 startpos; // start position
        math::vector3 endpos;   // final position
        cplane_t plane;         // surface normal at impact

        float fraction; // time completed, 1.0 = didn't hit anything

        int contents;             // contents on other side of surface hit
        unsigned short dispFlags; // displacement flags for marking surfaces with data

        bool allsolid;   // if true, plane is not valid
        bool startsolid; // if true, the initial point was in a solid area
    };

    class CGameTrace : public CBaseTrace
    {
      public:
        CGameTrace() = default;

        // auto DidHitWorld( ) const -> bool;
        // auto DidHitNonWorldEntity( ) const -> bool;
        // auto GetEntityIndex( ) const -> int;

        bool DidHit() const;
        bool IsVisible() const;

        float fractionleftsolid;          // time we left a solid, only valid if we started in solid
        csurface_t surface;               // surface hit (impact surface)
        int hitgroup;                     // 0 == generic, non-zero is specific body part
        short physicsbone;                // physics bone hit by trace in studio
        unsigned short worldSurfaceIndex; // Index of the msurface2_t, if applicable
        client_entity* hit_entity;
        int hitbox; // box hit by trace in studio
    };

    class IEngineTrace
    {
      public:
        virtual int GetPointContents(const math::vector3& vecAbsPosition, int contentsMask = MASK_ALL, handle_entity** ppEntity = nullptr) = 0;
        virtual int GetPointContents_WorldOnly(const math::vector3& vecAbsPosition, int contentsMask = MASK_ALL)                           = 0;
        virtual int GetPointContents_Collideable(collideable* pCollide, const math::vector3& vecAbsPosition)                               = 0;
        virtual void ClipRayToEntity(const Ray_t& ray, unsigned int fMask, handle_entity* pEnt, CGameTrace* pTrace)                        = 0;
        virtual void ClipRayToCollideable(const Ray_t& ray, unsigned int fMask, collideable* pCollide, CGameTrace* pTrace)                 = 0;
        virtual void TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, CGameTrace* pTrace)                        = 0;
    };
} // namespace fd::csgo
