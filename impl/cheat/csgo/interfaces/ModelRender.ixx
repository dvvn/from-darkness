module;

#include <cstdint>

export module cheat.csgo.interfaces.ModelRender;
export import cheat.csgo.math.Vector;
export import cheat.csgo.math.Qangle;
export import cheat.csgo.math.Vmatrix;
import nstd.one_instance;

export namespace cheat::csgo
{
	typedef unsigned short ModelInstanceHandle_t;
	typedef void* LightCacheHandle_t;
	typedef void* StudioDecalHandle_t;
	typedef int ImageFormat;

	class IClientRenderable;
	class IMaterial;
	class CStudioHdr;
	class IMatRenderContext;
	class DataCacheHandle_t;
	class ITexture;
	class KeyValues;
	class studiohdr_t;

	struct mstudioanimdesc_t;
	struct mstudioseqdesc_t;
	class Ray_t;
	struct DrawModelInfo_t;
	struct studiohwdata_t;
	struct MaterialLightingState_t;
	struct ColorMeshInfo_t;
	class model_t;

	enum class OverrideType_t :uint32_t
	{
		NORMAL = 0,
		BUILD_SHADOWS,
		DEPTH_WRITE,
		SELECTIVE,
		SSAO_DEPTH_WRITE
	};

	struct DrawModelState_t
	{
		studiohdr_t* m_pStudioHdr;
		studiohwdata_t* m_pStudioHWData;
		IClientRenderable* m_pRenderable;
		const matrix3x4_t* m_pModelToWorld;
		StudioDecalHandle_t m_decals;
		int m_drawFlags;
		int m_lod;
	};

	struct StaticPropRenderInfo_t
	{
		const matrix3x4_t* pModelToWorld;
		const model_t* pModel;
		IClientRenderable* pRenderable;
		Vector* pLightingOrigin;
		int16_t skin;
		ModelInstanceHandle_t instance;
	};

	struct ModelRenderInfo_t
	{
		Vector origin;
		QAngle angles;
		char pad[4];
		IClientRenderable* pRenderable;
		const model_t* pModel;
		const matrix3x4_t* pModelToWorld;
		const matrix3x4_t* pLightingOffset;
		const Vector* pLightingOrigin;
		int flags;
		int entity_index;
		int skin;
		int body;
		int hitboxset;
		ModelInstanceHandle_t instance;
	};

	struct LightingQuery_t
	{
		Vector m_LightingOrigin;
		ModelInstanceHandle_t m_InstanceHandle;
		bool m_bAmbientBoost;
	};

	struct StaticLightingQuery_t : LightingQuery_t
	{
		IClientRenderable* m_pRenderable;
	};

	class IVModelRender:public nstd::one_instance<IVModelRender*>
	{
	public:
		virtual int DrawModel(int flags, IClientRenderable* pRenderable, ModelInstanceHandle_t instance, int entity_index, const model_t* model, const Vector& origin,
							  const QAngle& angles,
							  int skin, int body, int hitboxset, const matrix3x4_t* modelToWorld = 0, const matrix3x4_t* pLightingOffset = 0) = 0;
		virtual void ForcedMaterialOverride(IMaterial* newMaterial, OverrideType_t nOverrideType = OverrideType_t::NORMAL, int nOverrides = 0) = 0;
		virtual bool IsForcedMaterialOverride( ) = 0;
		virtual void SetViewTarget(const CStudioHdr* pStudioHdr, int nBodyIndex, const Vector& target) = 0;
		virtual ModelInstanceHandle_t CreateInstance(IClientRenderable* pRenderable, LightCacheHandle_t* pCache = 0) = 0;
		virtual void DestroyInstance(ModelInstanceHandle_t handle) = 0;
		virtual void SetStaticLighting(ModelInstanceHandle_t handle, LightCacheHandle_t* pHandle) = 0;
		virtual LightCacheHandle_t GetStaticLighting(ModelInstanceHandle_t handle) = 0;
		virtual bool ChangeInstance(ModelInstanceHandle_t handle, IClientRenderable* pRenderable) = 0;
		virtual void AddDecal(ModelInstanceHandle_t handle, const Ray_t& ray, const Vector& decalUp, int decalIndex, int body, bool noPokeThru, int maxLODToDecal) = 0;
		virtual void RemoveAllDecals(ModelInstanceHandle_t handle) = 0;
		virtual bool ModelHasDecals(ModelInstanceHandle_t handle) = 0;
		virtual void RemoveAllDecalsFromAllModels( ) = 0;
		virtual matrix3x4_t* DrawModelShadowSetup(IClientRenderable* pRenderable, int body, int skin, DrawModelInfo_t* pInfo, matrix3x4_t* pCustomBoneToWorld = 0) = 0;
		virtual void DrawModelShadow(IClientRenderable* pRenderable, const DrawModelInfo_t& info, matrix3x4_t* pCustomBoneToWorld = 0) = 0;
		virtual bool RecomputeStaticLighting(ModelInstanceHandle_t handle) = 0;
		virtual void ReleaseAllStaticPropColorData( ) = 0;
		virtual void RestoreAllStaticPropColorData( ) = 0;
		virtual int DrawModelEx(ModelRenderInfo_t& pInfo) = 0;
		virtual int DrawModelExStaticProp(ModelRenderInfo_t& pInfo) = 0;
		virtual bool DrawModelSetup(ModelRenderInfo_t& pInfo, DrawModelState_t* pState, matrix3x4_t** ppBoneToWorldOut) = 0;
		virtual void DrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld = 0) = 0;
		virtual void SetupLighting(const Vector& vecCenter) = 0;
		virtual int DrawStaticPropArrayFast(StaticPropRenderInfo_t* pProps, int count, bool bShadowDepth) = 0;
		virtual void SuppressEngineLighting(bool bSuppress) = 0;
		virtual void SetupColorMeshes(int nTotalVerts) = 0;
		virtual void SetupLightingEx(const Vector& vecCenter, ModelInstanceHandle_t handle) = 0;
		virtual bool GetBrightestShadowingLightSource(const Vector& vecCenter, Vector& lightPos, Vector& lightBrightness, bool bAllowNonTaggedLights) = 0;
		virtual void ComputeLightingState(int nCount, const LightingQuery_t* pQuery, MaterialLightingState_t* pState, ITexture** ppEnvCubemapTexture) = 0;
		virtual void GetModelDecalHandles(StudioDecalHandle_t* pDecals, int nDecalStride, int nCount, const ModelInstanceHandle_t* pHandles) = 0;
		virtual void ComputeStaticLightingState(int nCount, const StaticLightingQuery_t* pQuery, MaterialLightingState_t* pState, MaterialLightingState_t* pDecalState,
												ColorMeshInfo_t** ppStaticLighting, ITexture** ppEnvCubemapTexture, DataCacheHandle_t* pColorMeshHandles) = 0;
		virtual void CleanupStaticLightingState(int nCount, DataCacheHandle_t* pColorMeshHandles) = 0;
	};

}
