#pragma once

#include <fd/valve/matrixX.h>
#include <fd/valve/qangle.h>
#include <fd/valve/vectorX.h>

#include <cstdint>

namespace fd::valve
{
    struct client_unknown;
    class model_t;

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

} // namespace fd::valve