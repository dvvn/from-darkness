module;

export module fds.csgo.interfaces.Collideable;
export import fds.math.qangle;
export import fds.math.vector3;
export import fds.math.matrix3x4;

export namespace fds::csgo
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

    class Ray_t;
    struct model_t;
    class CGameTrace;
    class IClientUnknown;
    class IHandleEntity;

    typedef CGameTrace trace_t;

    class ICollideable
    {
      public:
        virtual IHandleEntity* GetEntityHandle()                                                               = 0;
        virtual const math::vector3& OBBMins() const                                                           = 0;
        virtual const math::vector3& OBBMaxs() const                                                           = 0;
        virtual void WorldSpaceTriggerBounds(math::vector3* pVecWorldMins, math::vector3* pVecWorldMaxs) const = 0;
        virtual bool TestCollision(const Ray_t& ray, unsigned int fContentsMask, trace_t& tr)                  = 0;
        virtual bool TestHitboxes(const Ray_t& ray, unsigned int fContentsMask, trace_t& tr)                   = 0;
        virtual int GetCollisionModelIndex()                                                                   = 0;
        virtual const model_t* GetCollisionModel()                                                             = 0;
        virtual const math::vector3& GetCollisionOrigin() const                                                = 0;
        virtual const math::qangle& GetCollisionAngles() const                                                 = 0;
        virtual const math::matrix3x4& CollisionToWorldTransform() const                                       = 0;
        virtual SolidType_t GetSolid() const                                                                   = 0;
        virtual int GetSolidFlags() const                                                                      = 0;
        virtual IClientUnknown* GetIClientUnknown()                                                            = 0;
        virtual int GetCollisionGroup() const                                                                  = 0;
        virtual void WorldSpaceSurroundingBounds(math::vector3* pVecMins, math::vector3* pVecMaxs)             = 0;
        virtual bool ShouldTouchTrigger(int triggerSolidFlags) const                                           = 0;
        virtual const math::matrix3x4* GetRootParentToWorldTransform() const                                   = 0;
    };
} // namespace fds::csgo
