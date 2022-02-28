module;

#include <cstdint>

export module cheat.csgo.interfaces.C_BaseEntity;
export import cheat.csgo.interfaces.ClientEntity;
export import cheat.csgo.structs.Recv;
export import cheat.csgo.structs.DataMap;
export import cheat.csgo.structs.VarMapping;
export import cheat.csgo.math.Vector;
export import cheat.csgo.math.Qangle;
export import cheat.csgo.math.Vmatrix;
export import cheat.csgo.math.Quaternion;

export namespace cheat::csgo
{
	using string_t = const char*;
	// ReSharper disable once CppInconsistentNaming
	using HSCRIPT = uint32_t;

	enum class m_iTeamNum_t :int32_t
	{
		UNKNOWN = 0
		, SPEC = 1
		, T = 2
		, CT = 3
		,
	};

	enum class m_MoveType_t :int32_t
	{
		MOVETYPE_NONE = 0
		, MOVETYPE_ISOMETRIC
		, MOVETYPE_WALK
		, MOVETYPE_STEP
		, MOVETYPE_FLY
		, MOVETYPE_FLYGRAVITY
		, MOVETYPE_VPHYSICS
		, MOVETYPE_PUSH
		, MOVETYPE_NOCLIP
		, MOVETYPE_LADDER
		, MOVETYPE_OBSERVER
		, MOVETYPE_CUSTOM
		, MOVETYPE_LAST = MOVETYPE_CUSTOM
		,
		//MOVETYPE_MAX_BITS = 4,
		//MAX_MOVETYPE
	};

	enum class m_iEFlags_t :int32_t
	{
		EFL_KILLME = 1 << 0
		,
		// This entity is marked for death -- This allows the game to actually delete ents at a safe time
		EFL_DORMANT = 1 << 1
		,
		// Entity is dormant, no updates to client
		EFL_NOCLIP_ACTIVE = 1 << 2
		,
		// Lets us know when the noclip command is active.
		EFL_SETTING_UP_BONES = 1 << 3
		,
		// Set while a model is setting up its bones.
		EFL_KEEP_ON_RECREATE_ENTITIES = 1 << 4
		,
		// This is a special entity that should not be deleted when we restart entities only

		EFL_DIRTY_SHADOWUPDATE = 1 << 5
		,
		// Client only- need shadow manager to update the shadow...
		EFL_NOTIFY = 1 << 6
		,
		// Another entity is watching events on this entity (used by teleport)

		// The default behavior in ShouldTransmit is to not send an entity if it doesn't
		// have a model. Certain entities want to be sent anyway because all the drawing logic
		// is in the client DLL. They can set this flag and the engine will transmit them even
		// if they don't have a model.
		EFL_FORCE_CHECK_TRANSMIT = 1 << 7
		, EFL_BOT_FROZEN = 1 << 8
		,
		// This is set on bots that are frozen.
		EFL_SERVER_ONLY = 1 << 9
		,
		// Non-networked entity.
		EFL_NO_AUTO_EDICT_ATTACH = 1 << 10
		,
		// Don't attach the edict; we're doing it explicitly

		// Some dirty bits with respect to abs computations
		EFL_DIRTY_ABSTRANSFORM = 1 << 11
		, EFL_DIRTY_ABSVELOCITY = 1 << 12
		, EFL_DIRTY_ABSANGVELOCITY = 1 << 13
		, EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS = 1 << 14
		, EFL_DIRTY_SPATIAL_PARTITION = 1 << 15
		, EFL_HAS_PLAYER_CHILD = 1 << 16
		,
		// One of the child entities is a player.

		EFL_IN_SKYBOX = 1 << 17
		,
		// This is set if the entity detects that it's in the skybox.
		// This forces it to pass the "in PVS" for transmission.
		EFL_USE_PARTITION_WHEN_NOT_SOLID = 1 << 18
		,
		// Entities with this flag set show up in the partition even when not solid
		EFL_TOUCHING_FLUID = 1 << 19
		,
		// Used to determine if an entity is floating

		// FIXME: Not really sure where I should add this...
		EFL_IS_BEING_LIFTED_BY_BARNACLE = 1 << 20
		, EFL_NO_ROTORWASH_PUSH = 1 << 21
		,
		// I shouldn't be pushed by the rotorwash
		EFL_NO_THINK_FUNCTION = 1 << 22
		, EFL_NO_GAME_PHYSICS_SIMULATION = 1 << 23
		, EFL_CHECK_UNTOUCH = 1 << 24
		, EFL_DONTBLOCKLOS = 1 << 25
		,
		// I shouldn't block NPC line-of-sight
		EFL_DONTWALKON = 1 << 26
		,
		// NPC;s should not walk on this entity
		EFL_NO_DISSOLVE = 1 << 27
		,
		// These guys shouldn't dissolve
		EFL_NO_MEGAPHYSCANNON_RAGDOLL = 1 << 28
		,
		// Mega physcannon can't ragdoll these guys.
		EFL_NO_WATER_VELOCITY_CHANGE = 1 << 29
		,
		// Don't adjust this entity's velocity when transitioning into water
		EFL_NO_PHYSCANNON_INTERACTION = 1 << 30
		,
		// Physcannon can't pick these up or punt them
		EFL_NO_DAMAGE_FORCES = 1 << 31
		,
		// Doesn't accept forces from physics damage
	};

	enum class m_fEffects_t :uint32_t
	{
		EF_BONEMERGE = 0x001
		,
		// Performs bone merge on client side
		EF_BRIGHTLIGHT = 0x002
		,
		// DLIGHT centered at entity origin
		EF_DIMLIGHT = 0x004
		,
		// player flashlight
		EF_NOINTERP = 0x008
		,
		// don't interpolate the next frame
		EF_NOSHADOW = 0x010
		,
		// Don't cast no shadow
		EF_NODRAW = 0x020
		,
		// don't draw entity
		EF_NORECEIVESHADOW = 0x040
		,
		// Don't receive no shadow
		EF_BONEMERGE_FASTCULL = 0x080
		,
		// For use with EF_BONEMERGE. If this is set, then it places this ent's origin at its
		// parent and uses the parent's bbox + the max extents of the aiment.
		// Otherwise, it sets up the parent's bones every frame to figure out where to place
		// the aiment, which is inefficient because it'll setup the parent's bones even if
		// the parent is not in the PVS.
		EF_ITEM_BLINK = 0x100
		,
		// blink an item so that the user notices it.
		EF_PARENT_ANIMATES = 0x200
		,
		// always assume that the parent entity is animating
		EF_MARKED_FOR_FAST_REFLECTION = 0x400
		,
		// marks an entity for reflection rendering when using $reflectonlymarkedentities material variable
		EF_NOSHADOWDEPTH = 0x800
		,
		// Indicates this entity does not render into any shadow depthmap
		EF_SHADOWDEPTH_NOCACHE = 0x1000
		,
		// Indicates this entity cannot be cached in shadow depthmap and should render every frame
		EF_NOFLASHLIGHT = 0x2000
		, EF_NOCSM = 0x4000
		,
		// Indicates this entity does not render into the cascade shadow depthmap
		//EF_MAX_BITS = 15
	};

	class C_BaseEntity : public IClientEntity, public IClientModelRenderable
	{
	public:

#if __has_include("C_BaseEntity_generated_h")
#include "C_BaseEntity_generated_h"
#endif

		datamap_t* GetDataDescMap( );
		datamap_t* GetPredictionDescMap( );
	};
}
