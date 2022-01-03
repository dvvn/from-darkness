module;

#include <string_view>

export module cheat.csgo.interfaces:MaterialSystem;
export import :AppSystem;
export import cheat.csgo.math;

export namespace cheat::csgo
{
	//#define DECLARE_POINTER_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
	_INLINE_VAR constexpr auto MAXSTUDIOSKINS = 32;

	// These are given to FindMaterial to reference the texture groups that Show up on the 
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_LIGHTMAP = "Lightmaps";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_WORLD = "World textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_MODEL = "Model textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_VGUI = "VGUI textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_PARTICLE = "Particle textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_DECAL = "Decal textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_SKYBOX = "SkyBox textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_CLIENT_EFFECTS = "ClientEffect textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_OTHER = "Other textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_PRECACHED = "Precached";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_CUBE_MAP = "CubeMap textures";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_RENDER_TARGET = "RenderTargets";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_UNACCOUNTED = "Unaccounted textures";
	// _INLINE_VAR constexpr std::string_view TEXTURE_GROUP_STATIC_VERTEX_BUFFER	=	  "Static Vertex";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_STATIC_INDEX_BUFFER = "Static Indices";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP = "Displacement Verts";
	_INLINE_VAR	constexpr std::string_view TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR = "Lighting Verts";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD = "World Verts";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS = "Model Verts";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER = "Other Verts";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER = "Dynamic Indices";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER = "Dynamic Verts";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_DEPTH_BUFFER = "DepthBuffer";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_VIEW_MODEL = "ViewModel";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_PIXEL_SHADERS = "Pixel Shaders";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_VERTEX_SHADERS = "Vertex Shaders";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_RENDER_TARGET_SURFACE = "RenderTarget Surfaces";
	_INLINE_VAR constexpr std::string_view TEXTURE_GROUP_MORPH_TARGETS = "Morph Targets";

	//-----------------------------------------------------------------------------
	// forward declarations
	//-----------------------------------------------------------------------------

	class IMesh;
	class IVertexBuffer;
	class IIndexBuffer;
	struct MaterialSystem_Config_t;
	class ITexture;
	struct MaterialSystemHWID_t;
	class KeyValues;
	class IShader;
	class IVertexTexture;
	class IMorph;
	class IMatRenderContext;
	class ICallQueue;
	struct MorphWeight_t;
	class IFileList;
	struct VertexStreamSpec_t;
	struct ShaderStencilState_t;
	struct MeshInstanceData_t;
	class IClientMaterialSystem;
	class CPaintMaterial;
	class IPaintMapDataManager;
	class IPaintMapTextureManager;
	class GPUMemoryStats;
	struct AspectRatioInfo_t;
	struct CascadedShadowMappingState_t;

	class IMaterialProxyFactory;
	class ITexture;
	class IMaterialSystemHardwareConfig;
	class CShadowMgr;

	typedef int ImageFormat;

	enum CompiledVtfFlags
	{
		TEXTUREFLAGS_POINTSAMPLE = 0x00000001,
		TEXTUREFLAGS_TRILINEAR = 0x00000002,
		TEXTUREFLAGS_CLAMPS = 0x00000004,
		TEXTUREFLAGS_CLAMPT = 0x00000008,
		TEXTUREFLAGS_ANISOTROPIC = 0x00000010,
		TEXTUREFLAGS_HINT_DXT5 = 0x00000020,
		TEXTUREFLAGS_PWL_CORRECTED = 0x00000040,
		TEXTUREFLAGS_NORMAL = 0x00000080,
		TEXTUREFLAGS_NOMIP = 0x00000100,
		TEXTUREFLAGS_NOLOD = 0x00000200,
		TEXTUREFLAGS_ALL_MIPS = 0x00000400,
		TEXTUREFLAGS_PROCEDURAL = 0x00000800,
		TEXTUREFLAGS_ONEBITALPHA = 0x00001000,
		TEXTUREFLAGS_EIGHTBITALPHA = 0x00002000,
		TEXTUREFLAGS_ENVMAP = 0x00004000,
		TEXTUREFLAGS_RENDERTARGET = 0x00008000,
		TEXTUREFLAGS_DEPTHRENDERTARGET = 0x00010000,
		TEXTUREFLAGS_NODEBUGOVERRIDE = 0x00020000,
		TEXTUREFLAGS_SINGLECOPY = 0x00040000,
		TEXTUREFLAGS_PRE_SRGB = 0x00080000,
		TEXTUREFLAGS_UNUSED_00100000 = 0x00100000,
		TEXTUREFLAGS_UNUSED_00200000 = 0x00200000,
		TEXTUREFLAGS_UNUSED_00400000 = 0x00400000,
		TEXTUREFLAGS_NODEPTHBUFFER = 0x00800000,
		TEXTUREFLAGS_UNUSED_01000000 = 0x01000000,
		TEXTUREFLAGS_CLAMPU = 0x02000000,
		TEXTUREFLAGS_VERTEXTEXTURE = 0x04000000,
		TEXTUREFLAGS_SSBUMP = 0x08000000,
		TEXTUREFLAGS_UNUSED_10000000 = 0x10000000,
		TEXTUREFLAGS_BORDER = 0x20000000,
		TEXTUREFLAGS_UNUSED_40000000 = 0x40000000,
		TEXTUREFLAGS_UNUSED_80000000 = 0x80000000
	};

	enum StandardLightmap_t
	{
		MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE = -1,
		MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE_BUMP = -2,
		MATERIAL_SYSTEM_LIGHTMAP_PAGE_USER_DEFINED = -3
	};

	class IMaterial;
	struct MaterialSystem_SortInfo_t
	{
		IMaterial* material;
		int        lightmapPageID;
	};

	enum MaterialThreadMode_t
	{
		MATERIAL_SINGLE_THREADED,
		MATERIAL_QUEUED_SINGLE_THREADED,
		MATERIAL_QUEUED_THREADED
	};

	enum MaterialContextType_t
	{
		MATERIAL_HARDWARE_CONTEXT,
		MATERIAL_QUEUED_CONTEXT,
		MATERIAL_NULL_CONTEXT
	};

	enum
	{
		MATERIAL_ADAPTER_NAME_LENGTH = 512
	};

	struct MaterialTextureInfo_t
	{
		int iExcludeInformation;
	};

	struct ApplicationPerformanceCountersInfo_t
	{
		float msMain;
		float msMST;
		float msGPU;
		float msFlip;
		float msTotal;
	};

	struct ApplicationInstantCountersInfo_t
	{
		uint32_t m_nCpuActivityMask;
		uint32_t m_nDeferredWordsAllocated;
	};
	struct MaterialAdapterInfo_t
	{
		char         m_pDriverName[MATERIAL_ADAPTER_NAME_LENGTH];
		unsigned int m_VendorID;
		unsigned int m_DeviceID;
		unsigned int m_SubSysID;
		unsigned int m_Revision;
		int          m_nDXSupportLevel; // This is the *preferred* dx support level
		int          m_nMinDXSupportLevel;
		int          m_nMaxDXSupportLevel;
		unsigned int m_nDriverVersionHigh;
		unsigned int m_nDriverVersionLow;
	};

	struct MaterialVideoMode_t
	{
		int         m_Width;       // if width and height are 0 and you select 
		int         m_Height;      // windowed mode, it'll use the window size
		ImageFormat m_Format;      // use ImageFormats (ignored for windowed mode)
		int         m_RefreshRate; // 0 == default (ignored for windowed mode)
	};
	enum HDRType_t
	{
		HDR_TYPE_NONE,
		HDR_TYPE_INTEGER,
		HDR_TYPE_FLOAT,
	};

	enum RestoreChangeFlags_t
	{
		MATERIAL_RESTORE_VERTEX_FORMAT_CHANGED = 0x1,
		MATERIAL_RESTORE_RELEASE_MANAGED_RESOURCES = 0x2,
	};

	enum RenderTargetSizeMode_t
	{
		RT_SIZE_NO_CHANGE = 0,
		RT_SIZE_DEFAULT = 1,
		RT_SIZE_PICMIP = 2,
		RT_SIZE_HDR = 3,
		RT_SIZE_FULL_FRAME_BUFFER = 4,
		RT_SIZE_OFFSCREEN = 5,
		RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP = 6
	};

	enum MaterialRenderTargetDepth_t
	{
		MATERIAL_RT_DEPTH_SHARED = 0x0,
		MATERIAL_RT_DEPTH_SEPARATE = 0x1,
		MATERIAL_RT_DEPTH_NONE = 0x2,
		MATERIAL_RT_DEPTH_ONLY = 0x3,
	};

	using MaterialBufferReleaseFunc_t = void(*)(int nChangeFlags); // see RestoreChangeFlags_t
	using MaterialBufferRestoreFunc_t = void(*)(int nChangeFlags); // see RestoreChangeFlags_t
	using ModeChangeCallbackFunc_t = void(*)();
	using EndFrameCleanupFunc_t = void(*)();
	using EndFramePriorToNextContextFunc_t = bool(*)();
	using OnLevelShutdownFunc_t = void(*)(void* pUserData);

	using MaterialHandle_t = unsigned short;
	//DECLARE_POINTER_HANDLE (MaterialLock_t);

	class IMaterialVar;
	typedef int VertexFormat_t;
	typedef int MaterialPropertyTypes_t;

	enum PreviewImageRetVal_t
	{
		MATERIAL_PREVIEW_IMAGE_BAD = 0,
		MATERIAL_PREVIEW_IMAGE_OK,
		MATERIAL_NO_PREVIEW_IMAGE,
	};

	enum MaterialVarFlags_t
	{
		MATERIAL_VAR_DEBUG = (1 << 0),
		MATERIAL_VAR_NO_DEBUG_OVERRIDE = (1 << 1),
		MATERIAL_VAR_NO_DRAW = (1 << 2),
		MATERIAL_VAR_USE_IN_FILLRATE_MODE = (1 << 3),
		MATERIAL_VAR_VERTEXCOLOR = (1 << 4),
		MATERIAL_VAR_VERTEXALPHA = (1 << 5),
		MATERIAL_VAR_SELFILLUM = (1 << 6),
		MATERIAL_VAR_ADDITIVE = (1 << 7),
		MATERIAL_VAR_ALPHATEST = (1 << 8),
		//MATERIAL_VAR_UNUSED = (1 << 9),
		MATERIAL_VAR_ZNEARER = (1 << 10),
		MATERIAL_VAR_MODEL = (1 << 11),
		MATERIAL_VAR_FLAT = (1 << 12),
		MATERIAL_VAR_NOCULL = (1 << 13),
		MATERIAL_VAR_NOFOG = (1 << 14),
		MATERIAL_VAR_IGNOREZ = (1 << 15),
		MATERIAL_VAR_DECAL = (1 << 16),
		//MATERIAL_VAR_ENVMAPSPHERE = (1 << 17), // OBSOLETE
		//MATERIAL_VAR_UNUSED = (1 << 18), // UNUSED
		//MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19), // OBSOLETE
		MATERIAL_VAR_BASEALPHAENVMAPMASK = (1 << 20),
		MATERIAL_VAR_TRANSLUCENT = (1 << 21),
		MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
		//MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23), // OBSOLETE
		MATERIAL_VAR_OPAQUETEXTURE = (1 << 24),
		//MATERIAL_VAR_ENVMAPMODE = (1 << 25), // OBSOLETE
		MATERIAL_VAR_SUPPRESS_DECALS = (1 << 26),
		MATERIAL_VAR_HALFLAMBERT = (1 << 27),
		MATERIAL_VAR_WIREFRAME = (1 << 28),
		MATERIAL_VAR_ALLOWALPHATOCOVERAGE = (1 << 29),
		MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY = (1 << 30),
		MATERIAL_VAR_VERTEXFOG = (1 << 31),
	};

	class IMaterial
	{
	public:
		virtual const char* GetName( ) const = 0;
		virtual const char* GetTextureGroupName( ) const = 0;
		virtual PreviewImageRetVal_t GetPreviewImageProperties(int* width, int* height, ImageFormat* imageFormat, bool* isTranslucent) const = 0;
		virtual PreviewImageRetVal_t GetPreviewImage(unsigned char* data, int width, int height, ImageFormat imageFormat) const = 0;
		virtual int GetMappingWidth( ) = 0;
		virtual int GetMappingHeight( ) = 0;
		virtual int GetNumAnimationFrames( ) = 0;
		virtual bool InMaterialPage( ) = 0;
		virtual void GetMaterialOffset(float* pOffset) = 0;
		virtual void GetMaterialScale(float* pScale) = 0;
		virtual IMaterial* GetMaterialPage( ) = 0;
		virtual IMaterialVar* FindVar(const char* varName, bool* found, bool complain = true) = 0;
		virtual void IncrementReferenceCount( ) = 0;
		virtual void DecrementReferenceCount( ) = 0;
		void AddRef( ) { IncrementReferenceCount( ); }
		void Release( ) { DecrementReferenceCount( ); }
		virtual int GetEnumerationID( ) const = 0;
		virtual void GetLowResColorSample(float s, float t, float* color) const = 0;
		virtual void RecomputeStateSnapshots( ) = 0;
		virtual bool IsTranslucent( ) = 0;
		virtual bool IsAlphaTested( ) = 0;
		virtual bool IsVertexLit( ) = 0;
		virtual VertexFormat_t GetVertexFormat( ) const = 0;
		virtual bool HasProxy( ) const = 0;
		virtual bool UsesEnvCubemap( ) = 0;
		virtual bool NeedsTangentSpace( ) = 0;
		virtual bool NeedsPowerOfTwoFrameBufferTexture(bool bCheckSpecificToThisFrame = true) = 0;
		virtual bool NeedsFullFrameBufferTexture(bool bCheckSpecificToThisFrame = true) = 0;
		virtual bool NeedsSoftwareSkinning( ) = 0;
		virtual void AlphaModulate(float alpha) = 0;
		virtual void ColorModulate(float r, float g, float b) = 0;
		virtual void SetMaterialVarFlag(MaterialVarFlags_t flag, bool on) = 0;
		virtual bool GetMaterialVarFlag(MaterialVarFlags_t flag) const = 0;
		virtual void GetReflectivity(Vector& reflect) = 0;
		virtual bool GetPropertyFlag(MaterialPropertyTypes_t type) = 0;
		virtual bool IsTwoSided( ) = 0;
		virtual void SetShader(const char* pShaderName) = 0;
		virtual int GetNumPasses( ) = 0;
		virtual int GetTextureMemoryBytes( ) = 0;
		virtual void Refresh( ) = 0;
		virtual bool NeedsLightmapBlendAlpha( ) = 0;
		virtual bool NeedsSoftwareLighting( ) = 0;
		virtual int ShaderParamCount( ) const = 0;
		virtual IMaterialVar** GetShaderParams( ) = 0;
		virtual bool IsErrorMaterial( ) const = 0;
		virtual void Unused( ) = 0;
		virtual float GetAlphaModulation( ) = 0;
		virtual void GetColorModulation(float* r, float* g, float* b) = 0;
		virtual bool IsTranslucentUnderModulation(float fAlphaModulation = 1.0f) const = 0;
		virtual IMaterialVar* FindVarFast(const char* pVarName, unsigned int* pToken) = 0;
		virtual void SetShaderAndParams(KeyValues* pKeyValues) = 0;
		virtual const char* GetShaderName( ) const = 0;
		virtual void DeleteIfUnreferenced( ) = 0;
		virtual bool IsSpriteCard( ) = 0;
		virtual void CallBindProxy(void* proxyData) = 0;
		virtual void RefreshPreservingMaterialVars( ) = 0;
		virtual bool WasReloadedFromWhitelist( ) = 0;
		virtual bool SetTempExcluded(bool bSet, int nExcludedDimensionLimit) = 0;
		virtual int GetReferenceCount( ) const = 0;
	};

	class IMaterialSystem : public IAppSystem
	{
	public:
		virtual CreateInterfaceFn Init(const char* pShaderAPIDLL, IMaterialProxyFactory* pMaterialProxyFactory, CreateInterfaceFn fileSystemFactory,
									   CreateInterfaceFn cvarFactory = 0) = 0;
		virtual void SetShaderAPI(const char* pShaderAPIDLL) = 0;
		virtual void SetAdapter(int nAdapter, int nFlags) = 0;
		virtual void ModInit( ) = 0;
		virtual void ModShutdown( ) = 0;
		virtual void SetThreadMode(MaterialThreadMode_t mode, int nServiceThread = -1) = 0;
		virtual MaterialThreadMode_t GetThreadMode( ) = 0;
		virtual void ExecuteQueued( ) = 0;
		virtual void OnDebugEvent(const char* pEvent) = 0;
		virtual IMaterialSystemHardwareConfig* GetHardwareConfig(const char* pVersion, int* returnCode) = 0;
		virtual void __unknown( ) = 0;
		virtual bool UpdateConfig(bool bForceUpdate) = 0; //20
		virtual bool OverrideConfig(const MaterialSystem_Config_t& config, bool bForceUpdate) = 0;
		virtual const MaterialSystem_Config_t& GetCurrentConfigForVideoCard( ) const = 0;
		virtual bool GetRecommendedConfigurationInfo(int nDXLevel, KeyValues* pKeyValues) = 0;
		virtual int GetDisplayAdapterCount( ) const = 0;
		virtual int GetCurrentAdapter( ) const = 0;
		virtual void GetDisplayAdapterInfo(int adapter, MaterialAdapterInfo_t& info) const = 0;
		virtual int GetModeCount(int adapter) const = 0;
		virtual void GetModeInfo(int adapter, int mode, MaterialVideoMode_t& info) const = 0;
		virtual void AddModeChangeCallBack(ModeChangeCallbackFunc_t func) = 0;
		virtual void GetDisplayMode(MaterialVideoMode_t& mode) const = 0; //30
		virtual bool SetMode(void* hwnd, const MaterialSystem_Config_t& config) = 0;
		virtual bool SupportsMSAAMode(int nMSAAMode) = 0;
		virtual const MaterialSystemHWID_t& GetVideoCardIdentifier( ) const = 0;
		virtual void SpewDriverInfo( ) const = 0;
		virtual void GetBackBufferDimensions(int& width, int& height) const = 0;
		virtual ImageFormat GetBackBufferFormat( ) const = 0;
		virtual const AspectRatioInfo_t& GetAspectRatioInfo( ) const = 0;
		virtual bool SupportsHDRMode(HDRType_t nHDRModede) = 0;
		virtual bool AddView(void* hwnd) = 0;
		virtual void RemoveView(void* hwnd) = 0; //40
		virtual void SetView(void* hwnd) = 0;
		virtual void BeginFrame(float frameTime) = 0;
		virtual void EndFrame( ) = 0;
		virtual void Flush(bool flushHardware = false) = 0;
		virtual unsigned int GetCurrentFrameCount( ) = 0;
		virtual void SwapBuffers( ) = 0;
		virtual void EvictManagedResources( ) = 0;
		virtual void ReleaseResources( ) = 0;
		virtual void ReacquireResources( ) = 0;
		virtual void AddReleaseFunc(MaterialBufferReleaseFunc_t func) = 0; //50
		virtual void RemoveReleaseFunc(MaterialBufferReleaseFunc_t func) = 0;
		virtual void AddRestoreFunc(MaterialBufferRestoreFunc_t func) = 0;
		virtual void RemoveRestoreFunc(MaterialBufferRestoreFunc_t func) = 0;
		virtual void AddEndFrameCleanupFunc(EndFrameCleanupFunc_t func) = 0;
		virtual void RemoveEndFrameCleanupFunc(EndFrameCleanupFunc_t func) = 0;
		virtual void OnLevelShutdown( ) = 0;
		virtual bool AddOnLevelShutdownFunc(OnLevelShutdownFunc_t func, void* pUserData) = 0;
		virtual bool RemoveOnLevelShutdownFunc(OnLevelShutdownFunc_t func, void* pUserData) = 0;
		virtual void OnLevelLoadingComplete( ) = 0;
		virtual void ResetTempHWMemory(bool bExitingLevel = false) = 0; //60
		virtual void HandleDeviceLost( ) = 0;
		virtual int ShaderCount( ) const = 0;
		virtual int GetShaders(int nFirstShader, int nMaxCount, IShader** ppShaderList) const = 0;
		virtual int ShaderFlagCount( ) const = 0;
		virtual const char* ShaderFlagName(int nIndex) const = 0;
		virtual void GetShaderFallback(const char* pShaderName, char* pFallbackShader, int nFallbackLength) = 0;
		virtual IMaterialProxyFactory* GetMaterialProxyFactory( ) = 0;
		virtual void SetMaterialProxyFactory(IMaterialProxyFactory* pFactory) = 0;
		virtual void EnableEditorMaterials( ) = 0;
		virtual void EnableGBuffers( ) = 0; //70
		virtual void SetInStubMode(bool bInStubMode) = 0;
		virtual void DebugPrintUsedMaterials(const char* pSearchSubString, bool bVerbose) = 0;
		virtual void DebugPrintUsedTextures( ) = 0;
		virtual void ToggleSuppressMaterial(const char* pMaterialName) = 0;
		virtual void ToggleDebugMaterial(const char* pMaterialName) = 0;
		virtual bool UsingFastClipping( ) = 0;
		virtual int StencilBufferBits( ) = 0; //number of bits per pixel in the stencil buffer
		virtual void UncacheAllMaterials( ) = 0;
		virtual void UncacheUnusedMaterials(bool bRecomputeStateSnapshots = false) = 0;
		virtual void CacheUsedMaterials( ) = 0; //80
		virtual void ReloadTextures( ) = 0;
		virtual void ReloadMaterials(const char* pSubString = 0) = 0;
		virtual IMaterial* CreateMaterial(const char* pMaterialName, KeyValues* pVMTKeyValues) = 0;
		virtual IMaterial* FindMaterial(const char* pMaterialName, const char* pTextureGroupName = nullptr, bool complain = true, const char* pComplainPrefix = 0) = 0;
		virtual void unk0( ) = 0;
		virtual MaterialHandle_t FirstMaterial( ) const = 0;
		virtual MaterialHandle_t NextMaterial(MaterialHandle_t h) const = 0;
		virtual MaterialHandle_t InvalidMaterial( ) const = 0;
		virtual IMaterial* GetMaterial(MaterialHandle_t h) const = 0;
		virtual int GetNumMaterials( ) const = 0;
		virtual ITexture* FindTexture(const char* pTextureName, const char* pTextureGroupName, bool complain = true) = 0;
		virtual bool IsTextureLoaded(const char* pTextureName) const = 0;
		virtual ITexture* CreateProceduralTexture(const char* pTextureName, const char* pTextureGroupName, int w, int h, ImageFormat fmt, int nFlags) = 0;
		virtual void BeginRenderTargetAllocation( ) = 0;
		virtual void EndRenderTargetAllocation( ) = 0; // Simulate an Alt-Tab in here, which causes a release/restore of all resources
		virtual ITexture* CreateRenderTargetTexture(int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED) = 0;
		virtual ITexture* CreateNamedRenderTargetTextureEx(const char* pRTName, int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat format,
														   MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED,
														   unsigned int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, unsigned int renderTargetFlags = 0) = 0;
		virtual ITexture* CreateNamedRenderTargetTexture(const char* pRTName, int                           w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat format,
														 MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED, bool bClampTexCoords = true, bool bAutoMipMap = false) = 0;
		virtual ITexture* CreateNamedRenderTargetTextureEx2(const char* pRTName, int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat format,
															MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED,
															unsigned int                textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, unsigned int renderTargetFlags = 0) = 0;
	};
}
