module;

#include <string_view>

export module fd.valve.material_system;
export import fd.valve.app_system;
export import fd.math.vector3;

using namespace fd::valve;
using namespace fd::math;

//#define DECLARE_POINTER_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
constexpr auto MAXSTUDIOSKINS = 32;

namespace texture_group
{

    // These are given to FindMaterial to reference the texture groups that Show up on the
    constexpr std::string_view LIGHTMAP                    = "Lightmaps";
    constexpr std::string_view WORLD                       = "World textures";
    constexpr std::string_view MODEL                       = "Model textures";
    constexpr std::string_view VGUI                        = "VGUI textures";
    constexpr std::string_view PARTICLE                    = "Particle textures";
    constexpr std::string_view DECAL                       = "Decal textures";
    constexpr std::string_view SKYBOX                      = "SkyBox textures";
    constexpr std::string_view CLIENT_EFFECTS              = "ClientEffect textures";
    constexpr std::string_view OTHER                       = "Other textures";
    constexpr std::string_view PRECACHED                   = "Precached";
    constexpr std::string_view CUBE_MAP                    = "CubeMap textures";
    constexpr std::string_view RENDER_TARGET               = "RenderTargets";
    constexpr std::string_view UNACCOUNTED                 = "Unaccounted textures";
    // constexpr std::string_view STATIC_VERTEX_BUFFER	=	  "Static Vertex";
    constexpr std::string_view STATIC_INDEX_BUFFER         = "Static Indices";
    constexpr std::string_view STATIC_VERTEX_BUFFER_DISP   = "Displacement Verts";
    constexpr std::string_view STATIC_VERTEX_BUFFER_COLOR  = "Lighting Verts";
    constexpr std::string_view STATIC_VERTEX_BUFFER_WORLD  = "World Verts";
    constexpr std::string_view STATIC_VERTEX_BUFFER_MODELS = "Model Verts";
    constexpr std::string_view STATIC_VERTEX_BUFFER_OTHER  = "Other Verts";
    constexpr std::string_view DYNAMIC_INDEX_BUFFER        = "Dynamic Indices";
    constexpr std::string_view DYNAMIC_VERTEX_BUFFER       = "Dynamic Verts";
    constexpr std::string_view DEPTH_BUFFER                = "DepthBuffer";
    constexpr std::string_view VIEW_MODEL                  = "ViewModel";
    constexpr std::string_view PIXEL_SHADERS               = "Pixel Shaders";
    constexpr std::string_view VERTEX_SHADERS              = "Vertex Shaders";
    constexpr std::string_view RENDER_TARGET_SURFACE       = "RenderTarget Surfaces";
    constexpr std::string_view MORPH_TARGETS               = "Morph Targets";

} // namespace texture_group

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
//
/* class IMesh;
class IVertexBuffer;
class IIndexBuffer;
struct MaterialSystem_Config_t;
class ITexture;
struct MaterialSystemHWID_t;
class key_values;
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
struct aspect_ratio_info;
struct CascadedShadowMappingState_t;

class IMaterialProxyFactory;
class ITexture;
class IMaterialSystemHardwareConfig;
class CShadowMgr; */

typedef int ImageFormat;

class material_var;
class key_values;

struct material
{
    typedef int vertex_format;

    enum preview_image
    {
        MATERIAL_PREVIEW_IMAGE_BAD = 0,
        MATERIAL_PREVIEW_IMAGE_OK,
        MATERIAL_NO_PREVIEW_IMAGE,
    };

    enum var_flags
    {
        MATERIAL_VAR_DEBUG                    = (1 << 0),
        MATERIAL_VAR_NO_DEBUG_OVERRIDE        = (1 << 1),
        MATERIAL_VAR_NO_DRAW                  = (1 << 2),
        MATERIAL_VAR_USE_IN_FILLRATE_MODE     = (1 << 3),
        MATERIAL_VAR_VERTEXCOLOR              = (1 << 4),
        MATERIAL_VAR_VERTEXALPHA              = (1 << 5),
        MATERIAL_VAR_SELFILLUM                = (1 << 6),
        MATERIAL_VAR_ADDITIVE                 = (1 << 7),
        MATERIAL_VAR_ALPHATEST                = (1 << 8),
        // MATERIAL_VAR_UNUSED = (1 << 9),
        MATERIAL_VAR_ZNEARER                  = (1 << 10),
        MATERIAL_VAR_MODEL                    = (1 << 11),
        MATERIAL_VAR_FLAT                     = (1 << 12),
        MATERIAL_VAR_NOCULL                   = (1 << 13),
        MATERIAL_VAR_NOFOG                    = (1 << 14),
        MATERIAL_VAR_IGNOREZ                  = (1 << 15),
        MATERIAL_VAR_DECAL                    = (1 << 16),
        // MATERIAL_VAR_ENVMAPSPHERE = (1 << 17), // OBSOLETE
        // MATERIAL_VAR_UNUSED = (1 << 18), // UNUSED
        // MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19), // OBSOLETE
        MATERIAL_VAR_BASEALPHAENVMAPMASK      = (1 << 20),
        MATERIAL_VAR_TRANSLUCENT              = (1 << 21),
        MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
        // MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23), // OBSOLETE
        MATERIAL_VAR_OPAQUETEXTURE            = (1 << 24),
        // MATERIAL_VAR_ENVMAPMODE = (1 << 25), // OBSOLETE
        MATERIAL_VAR_SUPPRESS_DECALS          = (1 << 26),
        MATERIAL_VAR_HALFLAMBERT              = (1 << 27),
        MATERIAL_VAR_WIREFRAME                = (1 << 28),
        MATERIAL_VAR_ALLOWALPHATOCOVERAGE     = (1 << 29),
        MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY  = (1 << 30),
        MATERIAL_VAR_VERTEXFOG                = (1 << 31),
    };

    typedef int property_type;

    virtual const char* GetName() const                                                                                           = 0;
    virtual const char* GetTextureGroupName() const                                                                               = 0;
    virtual preview_image GetPreviewImageProperties(int* width, int* height, ImageFormat* imageFormat, bool* isTranslucent) const = 0;
    virtual preview_image GetPreviewImage(unsigned char* data, int width, int height, ImageFormat imageFormat) const              = 0;
    virtual int GetMappingWidth()                                                                                                 = 0;
    virtual int GetMappingHeight()                                                                                                = 0;
    virtual int GetNumAnimationFrames()                                                                                           = 0;
    virtual bool InMaterialPage()                                                                                                 = 0;
    virtual void GetMaterialOffset(float* pOffset)                                                                                = 0;
    virtual void GetMaterialScale(float* pScale)                                                                                  = 0;
    virtual material* GetMaterialPage()                                                                                           = 0;
    virtual material_var* FindVar(const char* varName, bool* found, bool complain = true)                                         = 0;
    virtual void IncrementReferenceCount()                                                                                        = 0;
    virtual void DecrementReferenceCount()                                                                                        = 0;

    void AddRef()
    {
        IncrementReferenceCount();
    }

    void Release()
    {
        DecrementReferenceCount();
    }

    virtual int GetEnumerationID() const                                                 = 0;
    virtual void GetLowResColorSample(float s, float t, float* color) const              = 0;
    virtual void RecomputeStateSnapshots()                                               = 0;
    virtual bool IsTranslucent()                                                         = 0;
    virtual bool IsAlphaTested()                                                         = 0;
    virtual bool IsVertexLit()                                                           = 0;
    virtual vertex_format GetVertexFormat() const                                        = 0;
    virtual bool HasProxy() const                                                        = 0;
    virtual bool UsesEnvCubemap()                                                        = 0;
    virtual bool NeedsTangentSpace()                                                     = 0;
    virtual bool NeedsPowerOfTwoFrameBufferTexture(bool CheckSpecificToThisFrame = true) = 0;
    virtual bool NeedsFullFrameBufferTexture(bool CheckSpecificToThisFrame = true)       = 0;
    virtual bool NeedsSoftwareSkinning()                                                 = 0;
    virtual void AlphaModulate(float alpha)                                              = 0;
    virtual void ColorModulate(float r, float g, float b)                                = 0;
    virtual void SetMaterialVarFlag(var_flags flag, bool on)                             = 0;
    virtual bool GetMaterialVarFlag(var_flags flag) const                                = 0;
    virtual void GetReflectivity(vector3& reflect)                                       = 0;
    virtual bool GetPropertyFlag(property_type type)                                     = 0;
    virtual bool IsTwoSided()                                                            = 0;
    virtual void SetShader(const char* pShaderName)                                      = 0;
    virtual int GetNumPasses()                                                           = 0;
    virtual int GetTextureMemoryBytes()                                                  = 0;
    virtual void Refresh()                                                               = 0;
    virtual bool NeedsLightmapBlendAlpha()                                               = 0;
    virtual bool NeedsSoftwareLighting()                                                 = 0;
    virtual int ShaderParamCount() const                                                 = 0;
    virtual material_var** GetShaderParams()                                             = 0;
    virtual bool IsErrorMaterial() const                                                 = 0;
    virtual void Unused()                                                                = 0;
    virtual float GetAlphaModulation()                                                   = 0;
    virtual void GetColorModulation(float* r, float* g, float* b)                        = 0;
    virtual bool IsTranslucentUnderModulation(float fAlphaModulation = 1.0f) const       = 0;
    virtual material_var* FindVarFast(const char* pVarName, unsigned int* pToken)        = 0;
    virtual void SetShaderAndParams(key_values* pKeyValues)                              = 0;
    virtual const char* GetShaderName() const                                            = 0;
    virtual void DeleteIfUnreferenced()                                                  = 0;
    virtual bool IsSpriteCard()                                                          = 0;
    virtual void CallBindProxy(void* proxyData)                                          = 0;
    virtual void RefreshPreservingMaterialVars()                                         = 0;
    virtual bool WasReloadedFromWhitelist()                                              = 0;
    virtual bool SetTempExcluded(bool Set, int nExcludedDimensionLimit)                  = 0;
    virtual int GetReferenceCount() const                                                = 0;
};

struct aspect_ratio_info;

struct material_system : app_system
{
#if 0
    enum restore_change_flags
    {
        MATERIAL_RESTORE_VERTEX_FORMAT_CHANGED     = 0x1,
        MATERIAL_RESTORE_RELEASE_MANAGED_RESOURCES = 0x2,
    };

    using buffer_release_func    = void (*)(restore_change_flags change_flags);
    using buffer_restore_func    = void (*)(restore_change_flags change_flags);
    using mode_change_callback   = void (*)();
    using end_frame_cleanup_func = void (*)();
    using on_level_shutdown_func = void (*)(void* user_data);

    virtual create_interface_fn Init(const char* pShaderAPIDLL,
                                     IMaterialProxyFactory* pMaterialProxyFactory,
                                     create_interface_fn fileSystemFactory,
                                     create_interface_fn cvarFactory = 0) = 0;
    virtual void SetShaderAPI(const char* pShaderAPIDLL)                  = 0;
    virtual void SetAdapter(int nAdapter, int nFlags)                     = 0;
    virtual void ModInit()                                                = 0;
    virtual void ModShutdown()                                            = 0;

    enum thread_mode
    {
        MATERIAL_SINGLE_THREADED,
        MATERIAL_QUEUED_SINGLE_THREADED,
        MATERIAL_QUEUED_THREADED
    };

    virtual void SetThreadMode(thread_mode mode, int nServiceThread = -1) = 0;
    virtual thread_mode GetThreadMode()                                   = 0;

    virtual void ExecuteQueued()                                                                    = 0;
    virtual void OnDebugEvent(const char* pEvent)                                                   = 0;
    virtual IMaterialSystemHardwareConfig* GetHardwareConfig(const char* pVersion, int* returnCode) = 0;
    virtual void __unknown()                                                                        = 0;
    virtual bool UpdateConfig(bool ForceUpdate)                                                     = 0; // 20
    virtual bool OverrideConfig(const MaterialSystem_Config_t& config, bool ForceUpdate)            = 0;
    virtual const MaterialSystem_Config_t& GetCurrentConfigForVideoCard() const                     = 0;
    virtual bool GetRecommendedConfigurationInfo(int nDXLevel, key_values* pKeyValues)              = 0;
    virtual int GetDisplayAdapterCount() const                                                      = 0;
    virtual int GetCurrentAdapter() const                                                           = 0;

    struct display_adapter_info
    {
        char DriverName[/* MATERIAL_ADAPTER_NAME_LENGTH */ 512];
        unsigned int m_VendorID;
        unsigned int m_DeviceID;
        unsigned int m_SubSysID;
        unsigned int m_Revision;
        int DXSupportLevel; // This is the *preferred* dx support level
        int MinDXSupportLevel;
        int MaxDXSupportLevel;
        unsigned int DriverVersionHigh;
        unsigned int DriverVersionLow;
    };

    virtual void GetDisplayAdapterInfo(int adapter, display_adapter_info& info) const = 0;
    virtual int GetModeCount(int adapter) const                                       = 0;

    struct video_mode_type
    {
        int m_Width;          // if width and height are 0 and you select
        int m_Height;         // windowed mode, it'll use the window size
        ImageFormat m_Format; // use ImageFormats (ignored for windowed mode)
        int m_RefreshRate;    // 0 == default (ignored for windowed mode)
    };

    virtual void GetModeInfo(int adapter, int mode, video_mode_type& info) const = 0;
    virtual void AddModeChangeCallBack(mode_change_callback func)                = 0;
    virtual void GetDisplayMode(video_mode_type& mode) const                     = 0; // 30
    virtual bool SetMode(void* hwnd, const MaterialSystem_Config_t& config)      = 0;
    virtual bool SupportsMSAAMode(int nMSAAMode)                                 = 0;
    virtual const MaterialSystemHWID_t& GetVideoCardIdentifier() const           = 0;
    virtual void SpewDriverInfo() const                                          = 0;
    virtual void GetBackBufferDimensions(int& width, int& height) const          = 0;
    virtual ImageFormat GetBackBufferFormat() const                              = 0;
    virtual const aspect_ratio_info& GetAspectRatioInfo() const                  = 0;

    enum hdr_type
    {
        HDR_TYPE_NONE,
        HDR_TYPE_INTEGER,
        HDR_TYPE_FLOAT,
    };

    virtual bool SupportsHDRMode(hdr_type nHDRModede)                                                   = 0;
    virtual bool AddView(void* hwnd)                                                                    = 0;
    virtual void RemoveView(void* hwnd)                                                                 = 0; // 40
    virtual void SetView(void* hwnd)                                                                    = 0;
    virtual void BeginFrame(float frameTime)                                                            = 0;
    virtual void EndFrame()                                                                             = 0;
    virtual void Flush(bool flushHardware = false)                                                      = 0;
    virtual unsigned int GetCurrentFrameCount()                                                         = 0;
    virtual void SwapBuffers()                                                                          = 0;
    virtual void EvictManagedResources()                                                                = 0;
    virtual void ReleaseResources()                                                                     = 0;
    virtual void ReacquireResources()                                                                   = 0;
    virtual void AddReleaseFunc(buffer_release_func func)                                               = 0; // 50
    virtual void RemoveReleaseFunc(buffer_release_func func)                                            = 0;
    virtual void AddRestoreFunc(buffer_restore_func func)                                               = 0;
    virtual void RemoveRestoreFunc(buffer_restore_func func)                                            = 0;
    virtual void AddEndFrameCleanupFunc(end_frame_cleanup_func func)                                    = 0;
    virtual void RemoveEndFrameCleanupFunc(end_frame_cleanup_func func)                                 = 0;
    virtual void OnLevelShutdown()                                                                      = 0;
    virtual bool AddOnLevelShutdownFunc(on_level_shutdown_func func, void* user_data)                   = 0;
    virtual bool RemoveOnLevelShutdownFunc(on_level_shutdown_func func, void* user_data)                = 0;
    virtual void OnLevelLoadingComplete()                                                               = 0;
    virtual void ResetTempHWMemory(bool ExitingLevel = false)                                           = 0; // 60
    virtual void HandleDeviceLost()                                                                     = 0;
    virtual int ShaderCount() const                                                                     = 0;
    virtual int GetShaders(int nFirstShader, int nMaxCount, IShader** ppShaderList) const               = 0;
    virtual int ShaderFlagCount() const                                                                 = 0;
    virtual const char* ShaderFlagName(int nIndex) const                                                = 0;
    virtual void GetShaderFallback(const char* pShaderName, char* pFallbackShader, int nFallbackLength) = 0;
    virtual IMaterialProxyFactory* GetMaterialProxyFactory()                                            = 0;
    virtual void SetMaterialProxyFactory(IMaterialProxyFactory* pFactory)                               = 0;
    virtual void EnableEditorMaterials()                                                                = 0;
    virtual void EnableGBuffers()                                                                       = 0; // 70
    virtual void SetInStubMode(bool InStubMode)                                                         = 0;
    virtual void DebugPrintUsedMaterials(const char* pSearchSubString, bool Verbose)                    = 0;
    virtual void DebugPrintUsedTextures()                                                               = 0;
    virtual void ToggleSuppressMaterial(const char* pMaterialName)                                      = 0;
    virtual void ToggleDebugMaterial(const char* pMaterialName)                                         = 0;
    virtual bool UsingFastClipping()                                                                    = 0;
    virtual int StencilBufferBits()                                                                     = 0; // number of bits per pixel in the stencil buffer
    virtual void UncacheAllMaterials()                                                                  = 0;
    virtual void UncacheUnusedMaterials(bool RecomputeStateSnapshots = false)                           = 0;
    virtual void CacheUsedMaterials()                                                                   = 0; // 80
    virtual void ReloadTextures()                                                                       = 0;
    virtual void ReloadMaterials(const char* pSubString = 0)                                            = 0;
    virtual material* CreateMaterial(const char* pMaterialName, key_values* pVMTKeyValues)              = 0;
    virtual material* FindMaterial(const char* pMaterialName, const char* pTextureGroupName = nullptr, bool complain = true, const char* pComplainPrefix = 0) = 0;
    virtual void unk0()                                                                                                                                       = 0;

    using MaterialHandle_t = unsigned short;

    virtual MaterialHandle_t FirstMaterial() const                  = 0;
    virtual MaterialHandle_t NextMaterial(MaterialHandle_t h) const = 0;
    virtual MaterialHandle_t InvalidMaterial() const                = 0;
    virtual material* GetMaterial(MaterialHandle_t h) const         = 0;

    virtual int GetNumMaterials() const                                                                                                           = 0;
    virtual ITexture* FindTexture(const char* pTextureName, const char* pTextureGroupName, bool complain = true)                                  = 0;
    virtual bool IsTextureLoaded(const char* pTextureName) const                                                                                  = 0;
    virtual ITexture* CreateProceduralTexture(const char* pTextureName, const char* pTextureGroupName, int w, int h, ImageFormat fmt, int nFlags) = 0;
    virtual void BeginRenderTargetAllocation()                                                                                                    = 0;
    virtual void EndRenderTargetAllocation() = 0; // Simulate an Alt-Tab in here, which causes a release/restore of all resources

    enum render_target_size_mode
    {
        RT_SIZE_NO_CHANGE                    = 0,
        RT_SIZE_DEFAULT                      = 1,
        RT_SIZE_PICMIP                       = 2,
        RT_SIZE_HDR                          = 3,
        RT_SIZE_FULL_FRAME_BUFFER            = 4,
        RT_SIZE_OFFSCREEN                    = 5,
        RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP = 6
    };

    enum render_target_depth
    {
        MATERIAL_RT_DEPTH_SHARED   = 0x0,
        MATERIAL_RT_DEPTH_SEPARATE = 0x1,
        MATERIAL_RT_DEPTH_NONE     = 0x2,
        MATERIAL_RT_DEPTH_ONLY     = 0x3,
    };

    virtual ITexture* CreateRenderTargetTexture(int w, int h, render_target_size_mode size_mode, ImageFormat format, render_target_depth depth = MATERIAL_RT_DEPTH_SHARED) = 0;
    virtual ITexture* CreateNamedRenderTargetTextureEx(const char* pRTName,
                                                       int w,
                                                       int h,
                                                       render_target_size_mode size_mode,
                                                       ImageFormat format,
                                                       render_target_depth depth      = MATERIAL_RT_DEPTH_SHARED,
                                                       unsigned int textureFlags      = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
                                                       unsigned int renderTargetFlags = 0)                                                                                 = 0;
    virtual ITexture* CreateNamedRenderTargetTexture(const char* pRTName,
                                                     int w,
                                                     int h,
                                                     render_target_size_mode size_mode,
                                                     ImageFormat format,
                                                     render_target_depth depth = MATERIAL_RT_DEPTH_SHARED,
                                                     bool ClampTexCoords       = true,
                                                     bool AutoMipMap           = false)                                                                                              = 0;
    virtual ITexture* CreateNamedRenderTargetTextureEx2(const char* pRTName,
                                                        int w,
                                                        int h,
                                                        render_target_size_mode size_mode,
                                                        ImageFormat format,
                                                        render_target_depth depth      = MATERIAL_RT_DEPTH_SHARED,
                                                        unsigned int textureFlags      = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
                                                        unsigned int renderTargetFlags = 0)                                                                                = 0;

#endif
};

export namespace fd::valve
{
    namespace texture_group = ::texture_group;

    using ::material;
    using ::material_system;
} // namespace fd::valve
