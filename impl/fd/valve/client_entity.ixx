module;

#include <cstdint>

export module fd.valve.client_entity;
export import fd.valve.base_handle;
export import fd.math.vector3;
export import fd.math.qangle;
export import fd.math.matrix3x4;

using namespace fd::valve;
using namespace fd::math;

struct model_t;
struct client_unknown;

struct collideable
{
    enum SolidType_t
    {
        SOLID_NONE     = 0, // no solid model
        SOLID_BSP      = 1, // a BSP tree
        SOLID_BBOX     = 2, // an AABB
        SOLID_OBB      = 3, // an OBB (not implemented yet)
        SOLID_OBB_YAW  = 4, // an OBB, constrained so that it can only yaw
        SOLID_CUSTOM   = 5, // Always call into the entity for tests
        SOLID_VPHYSICS = 6, // solid vphysics object, get vcollide from the model and collide with that
        SOLID_LAST,
    };

    virtual handle_entity* GetEntityHandle()                                                    = 0;
    virtual const vector3& OBBMins() const                                                      = 0;
    virtual const vector3& OBBMaxs() const                                                      = 0;
    virtual void WorldSpaceTriggerBounds(vector3* pVecWorldMins, vector3* pVecWorldMaxs) const  = 0;
    virtual bool TestCollision(/* const Ray_t& ray, unsigned int fContentsMask, trace_t& tr */) = 0;
    virtual bool TestHitboxes(/* const Ray_t& ray, unsigned int fContentsMask, trace_t& tr */) = 0;
    virtual int GetCollisionModelIndex()                                                        = 0;
    virtual const model_t* GetCollisionModel()                                                  = 0;
    virtual const vector3& GetCollisionOrigin() const                                           = 0;
    virtual const qangle& GetCollisionAngles() const                                            = 0;
    virtual const matrix3x4& CollisionToWorldTransform() const                                  = 0;
    virtual SolidType_t GetSolid() const                                                        = 0;
    virtual int GetSolidFlags() const                                                           = 0;
    virtual client_unknown* GetIClientUnknown()                                                 = 0;
    virtual int GetCollisionGroup() const                                                       = 0;
    virtual void WorldSpaceSurroundingBounds(vector3* pVecMins, vector3* pVecMaxs)              = 0;
    virtual bool ShouldTouchTrigger(int triggerSolidFlags) const                                = 0;
    virtual const matrix3x4* GetRootParentToWorldTransform() const                              = 0;
};

struct client_networkable;
struct client_renderable;
struct client_entity;
struct base_entity;
struct client_thinkable;

struct client_unknown : handle_entity
{
    virtual collideable* GetCollideable()                             = 0;
    virtual client_networkable* GetClientNetworkable()                = 0;
    virtual client_renderable* GetClientRenderable()                  = 0;
    virtual client_entity* GetIClientEntity()                         = 0;
    virtual base_entity* GetBaseEntity()                              = 0;
    virtual client_thinkable* GetClientThinkable()                    = 0;
    // virtual IClientModelRenderable*  GetClientModelRenderable() = 0;
    virtual /* IClientAlphaProperty */ void* GetClientAlphaProperty() = 0;
};

typedef unsigned short ClientShadowHandle_t;
typedef unsigned short ClientRenderHandle_t;
typedef unsigned short ModelInstanceHandle_t;

struct client_renderable
{
    virtual client_unknown* GetIClientUnknown()                                                                                     = 0;
    virtual const vector3& GetRenderOrigin()                                                                                        = 0;
    virtual const qangle& GetRenderAngles()                                                                                         = 0;
    virtual bool ShouldDraw()                                                                                                       = 0;
    virtual int GetRenderFlags()                                                                                                    = 0; // ERENDERFLAGS_xxx
    virtual void Unused() const                                                                                                     = 0;
    virtual ClientShadowHandle_t GetShadowHandle() const                                                                            = 0;
    virtual ClientRenderHandle_t& RenderHandle()                                                                                    = 0;
    virtual const model_t* GetModel() const                                                                                         = 0;
    virtual int DrawModel(int flags, const int /*RenderableInstance_t*/& instance)                                                  = 0;
    virtual int GetBody()                                                                                                           = 0;
    virtual void GetColorModulation(float* color)                                                                                   = 0;
    virtual bool LODTest()                                                                                                          = 0;
    virtual bool SetupBones(matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)                             = 0;
    virtual void SetupWeights(const matrix3x4* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) = 0;
    virtual void DoAnimationEvents()                                                                                                = 0;
    virtual /* IPVSNotify */ void* GetPVSNotifyInterface()                                                                          = 0;
    virtual void GetRenderBounds(vector3& mins, vector3& maxs)                                                                      = 0;
    virtual void GetRenderBoundsWorldspace(vector3& mins, vector3& maxs)                                                            = 0;
    virtual void GetShadowRenderBounds(vector3& mins, vector3& maxs, int /*ShadowType_t*/ shadowType)                               = 0;
    virtual bool ShouldReceiveProjectedTextures(int flags)                                                                          = 0;
    virtual bool GetShadowCastDistance(float* pDist, int /*ShadowType_t*/ shadowType) const                                         = 0;
    virtual bool GetShadowCastDirection(vector3* pDirection, int /*ShadowType_t*/ shadowType) const                                 = 0;
    virtual bool IsShadowDirty()                                                                                                    = 0;
    virtual void MarkShadowDirty(bool Dirty)                                                                                        = 0;
    virtual client_renderable* GetShadowParent()                                                                                    = 0;
    virtual client_renderable* FirstShadowChild()                                                                                   = 0;
    virtual client_renderable* NextShadowPeer()                                                                                     = 0;
    virtual int /*ShadowType_t*/ ShadowCastType()                                                                                   = 0;
    virtual void CreateModelInstance()                                                                                              = 0;
    virtual ModelInstanceHandle_t GetModelInstance()                                                                                = 0;
    virtual const matrix3x4& RenderableToWorldTransform()                                                                           = 0;
    virtual int LookupAttachment(const char* pAttachmentName)                                                                       = 0;
    virtual bool GetAttachment(int number, vector3& origin, qangle& angles)                                                         = 0;
    virtual bool GetAttachment(int number, matrix3x4& matrix)                                                                       = 0;
    virtual float* GetRenderClipPlane()                                                                                             = 0;
    virtual int GetSkin()                                                                                                           = 0;
    virtual void OnThreadedDrawSetup()                                                                                              = 0;
    virtual bool UsesFlexDelayedWeights()                                                                                           = 0;
    virtual void RecordToolMessage()                                                                                                = 0;
    virtual bool ShouldDrawForSplitScreenUser(int nSlot)                                                                            = 0;
    virtual uint8_t OverrideAlphaModulation(uint8_t nAlpha)                                                                         = 0;
    virtual uint8_t OverrideShadowAlphaModulation(uint8_t nAlpha)                                                                   = 0;
};

class bf_read;
class client_class;

struct client_networkable
{
    virtual client_unknown* GetIClientUnknown()            = 0;
    virtual void Release()                                 = 0;
    virtual client_class* GetClientClass()                 = 0;
    virtual void NotifyShouldTransmit(int state)           = 0;
    virtual void OnPreDataChanged(int updateType)          = 0;
    virtual void OnDataChanged(int updateType)             = 0;
    virtual void PreDataUpdate(int updateType)             = 0;
    virtual void PostDataUpdate(int updateType)            = 0;
    virtual void __unkn()                                  = 0;
    virtual bool IsDormant()                               = 0;
    virtual int EntIndex() const                           = 0;
    virtual void ReceiveMessage(int classID, bf_read& msg) = 0;
    virtual void* GetDataTableBasePtr()                    = 0;
    virtual void SetDestroyedOnRecreateEntities()          = 0;
};

struct client_thinkable
{
    using handle_type = void*;

    virtual client_unknown* GetIClientUnknown()     = 0;
    virtual void ClientThink()                      = 0;
    virtual handle_type GetThinkHandle()            = 0;
    virtual void SetThinkHandle(handle_type hThink) = 0;
    virtual void Release()                          = 0;
};

struct client_entity : client_unknown, client_renderable, client_networkable, client_thinkable
{
    virtual const vector3& GetAbsOrigin() const                                 = 0;
    virtual const qangle& GetAbsAngles() const                                  = 0;
    virtual void* GetMouth()                                                    = 0;
    virtual bool GetSoundSpatialization(/* SpatializationInfo_t& info */ void*) = 0;
    virtual bool IsBlurred()                                                    = 0;
};

struct client_entity_list
{
    virtual client_networkable* GetClientNetworkable(int entnum)       = 0;
    virtual void* vtablepad0x1()                                       = 0;
    virtual void* vtablepad0x2()                                       = 0;
    virtual client_entity* GetClientEntity(int entNum)                 = 0;
    virtual client_entity* GetClientEntityFromHandle(base_handle hEnt) = 0;
    virtual int NumberOfEntities(bool IncludeNonNetworkable)           = 0;
    virtual int GetHighestEntityIndex()                                = 0;
    virtual void SetMaxEntities(int maxEnts)                           = 0;
    virtual int GetMaxEntities()                                       = 0;
};

export namespace fd::valve
{
    using ::client_entity;
    using ::client_entity_list;
} // namespace fd::valve
