module;

#include <cstdint>

export module fd.csgo.interfaces.ClientRenderable;
export import fd.math.qangle;
export import fd.math.vector3;
export import fd.math.matrix3x4;

export namespace fd::csgo
{
    using ClientShadowHandle_t  = unsigned short;
    using ClientRenderHandle_t  = unsigned short;
    using ModelInstanceHandle_t = unsigned short;
    // using uint8_t = unsigned char;

    struct model_t;
    class IPVSNotify;
    class IClientUnknown;

    class IClientRenderable
    {
      public:
        virtual IClientUnknown* GetIClientUnknown()                                                                                           = 0;
        virtual const math::vector3& GetRenderOrigin()                                                                                        = 0;
        virtual const math::qangle& GetRenderAngles()                                                                                         = 0;
        virtual bool ShouldDraw()                                                                                                             = 0;
        virtual int GetRenderFlags()                                                                                                          = 0; // ERENDERFLAGS_xxx
        virtual void Unused() const                                                                                                           = 0;
        virtual ClientShadowHandle_t GetShadowHandle() const                                                                                  = 0;
        virtual ClientRenderHandle_t& RenderHandle()                                                                                          = 0;
        virtual const model_t* GetModel() const                                                                                               = 0;
        virtual int DrawModel(int flags, const int /*RenderableInstance_t*/& instance)                                                        = 0;
        virtual int GetBody()                                                                                                                 = 0;
        virtual void GetColorModulation(float* color)                                                                                         = 0;
        virtual bool LODTest()                                                                                                                = 0;
        virtual bool SetupBones(math::matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)                             = 0;
        virtual void SetupWeights(const math::matrix3x4* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) = 0;
        virtual void DoAnimationEvents()                                                                                                      = 0;
        virtual IPVSNotify* GetPVSNotifyInterface()                                                                                           = 0;
        virtual void GetRenderBounds(math::vector3& mins, math::vector3& maxs)                                                                = 0;
        virtual void GetRenderBoundsWorldspace(math::vector3& mins, math::vector3& maxs)                                                      = 0;
        virtual void GetShadowRenderBounds(math::vector3& mins, math::vector3& maxs, int /*ShadowType_t*/ shadowType)                         = 0;
        virtual bool ShouldReceiveProjectedTextures(int flags)                                                                                = 0;
        virtual bool GetShadowCastDistance(float* pDist, int /*ShadowType_t*/ shadowType) const                                               = 0;
        virtual bool GetShadowCastDirection(math::vector3* pDirection, int /*ShadowType_t*/ shadowType) const                                 = 0;
        virtual bool IsShadowDirty()                                                                                                          = 0;
        virtual void MarkShadowDirty(bool bDirty)                                                                                             = 0;
        virtual IClientRenderable* GetShadowParent()                                                                                          = 0;
        virtual IClientRenderable* FirstShadowChild()                                                                                         = 0;
        virtual IClientRenderable* NextShadowPeer()                                                                                           = 0;
        virtual int /*ShadowType_t*/ ShadowCastType()                                                                                         = 0;
        virtual void CreateModelInstance()                                                                                                    = 0;
        virtual ModelInstanceHandle_t GetModelInstance()                                                                                      = 0;
        virtual const math::matrix3x4& RenderableToWorldTransform()                                                                           = 0;
        virtual int LookupAttachment(const char* pAttachmentName)                                                                             = 0;
        virtual bool GetAttachment(int number, math::vector3& origin, math::qangle& angles)                                                   = 0;
        virtual bool GetAttachment(int number, math::matrix3x4& matrix)                                                                       = 0;
        virtual float* GetRenderClipPlane()                                                                                                   = 0;
        virtual int GetSkin()                                                                                                                 = 0;
        virtual void OnThreadedDrawSetup()                                                                                                    = 0;
        virtual bool UsesFlexDelayedWeights()                                                                                                 = 0;
        virtual void RecordToolMessage()                                                                                                      = 0;
        virtual bool ShouldDrawForSplitScreenUser(int nSlot)                                                                                  = 0;
        virtual uint8_t OverrideAlphaModulation(uint8_t nAlpha)                                                                               = 0;
        virtual uint8_t OverrideShadowAlphaModulation(uint8_t nAlpha)                                                                         = 0;
    };

    class IClientModelRenderable
    {
      public:
        virtual bool GetRenderData(void* pData, /*ModelDataCategory_t*/ int nCategory) = 0;
    };
} // namespace fd::csgo
