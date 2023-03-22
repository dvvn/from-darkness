#pragma once
#include <fd/valve/entity_handle.h>
#include <fd/valve/matrix3x4.h>
#include <fd/valve/qangle.h>
#include <fd/valve/vector3d.h>

namespace fd::valve::client_side
{
struct unknown;
struct model_t;

struct collideable
{
    virtual entity_handle *GetEntityHandle()                                                  = 0;
    virtual vector3d const &OBBMins() const                                                   = 0;
    virtual vector3d const &OBBMaxs() const                                                   = 0;
    virtual void WorldSpaceTriggerBounds(vector3d *WorldMins, vector3d *WorldMaxs) const      = 0;
    virtual bool TestCollision(/*const Ray_t &ray, unsigned int fContentsMask, trace_t &tr*/) = 0;
    virtual bool TestHitboxes(/*const Ray_t &ray, unsigned int fContentsMask, trace_t &tr*/) = 0;
    virtual int GetCollisionModelIndex()                                                      = 0;
    virtual model_t const *GetCollisionModel()                                                = 0;
    virtual vector3d const &GetCollisionOrigin() const                                        = 0;
    virtual qangle const &GetCollisionAngles() const                                          = 0;
    virtual matrix3x4 const &CollisionToWorldTransform() const                                = 0;
    virtual enum SolidType_t GetSolid() const                                                 = 0;
    virtual int GetSolidFlags() const                                                         = 0;
    virtual unknown *GetIClientUnknown()                                                      = 0;
    virtual int GetCollisionGroup() const                                                     = 0;
    virtual void WorldSpaceSurroundingBounds(vector3d *Mins, vector3d *Maxs)                  = 0;
    virtual bool ShouldTouchTrigger(int triggerSolidFlags) const                              = 0;
    virtual matrix3x4 const *GetRootParentToWorldTransform() const                            = 0;
};
}