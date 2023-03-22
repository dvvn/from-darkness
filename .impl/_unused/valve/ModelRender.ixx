module;

#include <cstdint>

export module fd.ModelRender;
export import fd.math.vector3;
export import fd.math.qangle;
export import fd.math.matrix3x4;

export namespace fd::csgo
{
    typedef unsigned short ModelInstanceHandle_t;
    typedef void* LightCacheHandle_t;
    typedef void* StudioDecalHandle_t;
    typedef int ImageFormat;

    class client_renderable;
    class material;
    class CStudioHdr;
    class IMatRenderContext;
    class DataCacheHandle_t;
    class ITexture;
    class key_values;
    class studiohdr_t;

    struct mstudioanimdesc_t;
    struct mstudioseqdesc_t;
    class Ray_t;
    struct DrawModelInfo_t;
    struct studiohwdata_t;
    struct MaterialLightingState_t;
    struct ColorMeshInfo_t;
    class model_t;

    enum class OverrideType_t : uint32_t
    {
        NORMAL = 0,
        BUILD_SHADOWS,
        DEPTH_WRITE,
        SELECTIVE,
        SSAO_DEPTH_WRITE
    };

    struct DrawModelState_t
    {
        studiohdr_t* StudioHdr;
        studiohwdata_t* StudioHWData;
        client_renderable* Renderable;
        const math::matrix3x4* ModelToWorld;
        StudioDecalHandle_t m_decals;
        int m_drawFlags;
        int m_lod;
    };

    struct StaticPropRenderInfo_t
    {
        const math::matrix3x4* pModelToWorld;
        const model_t* pModel;
        client_renderable* pRenderable;
        math::vector3* pLightingOrigin;
        int16_t skin;
        ModelInstanceHandle_t instance;
    };

    struct ModelRenderInfo_t
    {
        math::vector3 origin;
        math::qangle angles;
        char pad[4];
        client_renderable* pRenderable;
        const model_t* pModel;
        const math::matrix3x4* pModelToWorld;
        const math::matrix3x4* pLightingOffset;
        const math::vector3* pLightingOrigin;
        int flags;
        int entity_index;
        int skin;
        int body;
        int hitboxset;
        ModelInstanceHandle_t instance;
    };

    struct LightingQuery_t
    {
        math::vector3 m_LightingOrigin;
        ModelInstanceHandle_t m_InstanceHandle;
        bool AmbientBoost;
    };

    struct StaticLightingQuery_t : LightingQuery_t
    {
        client_renderable* Renderable;
    };

    class IVModelRender
    {
      public:
        virtual int DrawModel(int flags,
                              client_renderable* pRenderable,
                              ModelInstanceHandle_t instance,
                              int entity_index,
                              const model_t* model,
                              const math::vector3& origin,
                              const math::qangle& angles,
                              int skin,
                              int body,
                              int hitboxset,
                              const math::matrix3x4* modelToWorld    = 0,
                              const math::matrix3x4* pLightingOffset = 0)                                                                                                  = 0;
        virtual void ForcedMaterialOverride(material* newMaterial, OverrideType_t nOverrideType = OverrideType_t::NORMAL, int nOverrides = 0)                              = 0;
        virtual bool IsForcedMaterialOverride()                                                                                                                            = 0;
        virtual void SetViewTarget(const CStudioHdr* pStudioHdr, int nBodyIndex, const math::vector3& target)                                                              = 0;
        virtual ModelInstanceHandle_t CreateInstance(client_renderable* pRenderable, LightCacheHandle_t* pCache = 0)                                                       = 0;
        virtual void DestroyInstance(ModelInstanceHandle_t handle)                                                                                                         = 0;
        virtual void SetStaticLighting(ModelInstanceHandle_t handle, LightCacheHandle_t* pHandle)                                                                          = 0;
        virtual LightCacheHandle_t GetStaticLighting(ModelInstanceHandle_t handle)                                                                                         = 0;
        virtual bool ChangeInstance(ModelInstanceHandle_t handle, client_renderable* pRenderable)                                                                          = 0;
        virtual void AddDecal(ModelInstanceHandle_t handle, const Ray_t& ray, const math::vector3& decalUp, int decalIndex, int body, bool noPokeThru, int maxLODToDecal)  = 0;
        virtual void RemoveAllDecals(ModelInstanceHandle_t handle)                                                                                                         = 0;
        virtual bool ModelHasDecals(ModelInstanceHandle_t handle)                                                                                                          = 0;
        virtual void RemoveAllDecalsFromAllModels()                                                                                                                        = 0;
        virtual math::matrix3x4* DrawModelShadowSetup(client_renderable* pRenderable, int body, int skin, DrawModelInfo_t* pInfo, math::matrix3x4* pCustomBoneToWorld = 0) = 0;
        virtual void DrawModelShadow(client_renderable* pRenderable, const DrawModelInfo_t& info, math::matrix3x4* pCustomBoneToWorld = 0)                                 = 0;
        virtual bool RecomputeStaticLighting(ModelInstanceHandle_t handle)                                                                                                 = 0;
        virtual void ReleaseAllStaticPropColorData()                                                                                                                       = 0;
        virtual void RestoreAllStaticPropColorData()                                                                                                                       = 0;
        virtual int DrawModelEx(ModelRenderInfo_t& pInfo)                                                                                                                  = 0;
        virtual int DrawModelExStaticProp(ModelRenderInfo_t& pInfo)                                                                                                        = 0;
        virtual bool DrawModelSetup(ModelRenderInfo_t& pInfo, DrawModelState_t* pState, math::matrix3x4** ppBoneToWorldOut)                                                = 0;
        virtual void DrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, math::matrix3x4* pCustomBoneToWorld = 0)      = 0;
        virtual void SetupLighting(const math::vector3& vecCenter)                                                                                                         = 0;
        virtual int DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool ShadowDepth)                                                                   = 0;
        virtual void SuppressEngineLighting(bool Suppress)                                                                                                                 = 0;
        virtual void SetupColorMeshes(int nTotalVerts)                                                                                                                     = 0;
        virtual void SetupLightingEx(const math::vector3& vecCenter, ModelInstanceHandle_t handle)                                                                         = 0;
        virtual bool GetBrightestShadowingLightSource(const math::vector3& vecCenter, math::vector3& lightPos, math::vector3& lightBrightness, bool AllowNonTaggedLights)  = 0;
        virtual void ComputeLightingState(int nCount, const LightingQuery_t* pQuery, MaterialLightingState_t* pState, ITexture** ppEnvCubemapTexture)                      = 0;
        virtual void GetModelDecalHandles(StudioDecalHandle_t* pDecals, int nDecalStride, int nCount, const ModelInstanceHandle_t* pHandles)                               = 0;
        virtual void ComputeStaticLightingState(int nCount,
                                                const StaticLightingQuery_t* pQuery,
                                                MaterialLightingState_t* pState,
                                                MaterialLightingState_t* pDecalState,
                                                ColorMeshInfo_t** ppStaticLighting,
                                                ITexture** ppEnvCubemapTexture,
                                                DataCacheHandle_t* pColorMeshHandles)                                                                                      = 0;
        virtual void CleanupStaticLightingState(int nCount, DataCacheHandle_t* pColorMeshHandles)                                                                          = 0;
    };

} // namespace fd::csgo
