module;

#include <array>
#include <cstdint>
#include <cstring>

export module fd.StudioRender;
export import fd.Studio;
export import fd.app_system;
export import fd.ModelRender;
export import fd.valve_vector;
export import fd.math.view_matrix;
export import fd.math.vector2;

export namespace fd::csgo
{
    class material_system;
    class material;
    class CUtlBuffer;
    class client_renderable;
    struct studiohwdata_t;
    class ITexture;
    struct ShaderStencilState_t;
    struct MeshInstanceData_t;

    typedef void* StudioDecalHandle_t;

    enum class DrawModelFlags_t : int32_t
    {
        DRAW_ENTIRE_MODEL                                = 0,
        DRAW_OPAQUE_ONLY                                 = 0x01,
        DRAW_TRANSLUCENT_ONLY                            = 0x02,
        DRAW_GROUP_MASK                                  = 0x03,
        DRAW_NO_FLEXES                                   = 0x04,
        DRAW_STATIC_LIGHTING                             = 0x08,
        DRAW_ACCURATETIME                                = 0x10,
        DRAW_NO_SHADOWS                                  = 0x20,
        DRAW_GET_PERF_STATS                              = 0x40,
        DRAW_WIREFRAME                                   = 0x80,
        DRAW_ITEM_BLINK                                  = 0x100,
        SHADOWDEPTHTEXTURE                               = 0x200,
        UNUSED                                           = 0x400,
        SKIP_DECALS                                      = 0x800,
        /*SSAODEPTHTEXTURE*/ MODEL_IS_CACHEABLE          = 0x1000,
        SHADOWDEPTHTEXTURE_INCLUDE_TRANSLUCENT_MATERIALS = 0x2000,
        NO_PRIMARY_DRAW                                  = 0x4000,
        /*GENERATE_STATS*/ SSAODEPTHTEXTURE              = 0x8000,
    };
    enum class LightType_t : int32_t
    {
        DISABLE = 0,
        POINT,
        DIRECTIONAL,
        SPOT,
    };

    enum LightType_OptimizationFlags_t
    {
        LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION0      = 1,
        LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION1      = 2,
        LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION2      = 4,
        LIGHTTYPE_OPTIMIZATIONFLAGS_DERIVED_VALUES_CALCED = 8,
    };

    struct LightDesc_t
    {
        LightType_t m_Type;        //< xxx
        math::vector3 m_Color;     //< color+intensity
        math::vector3 m_Position;  //< light source center position
        math::vector3 m_Direction; //< for SPOT, direction it is pointing
        float m_Range;             //< distance range for light.0=infinite
        float m_Falloff;           //< angular falloff exponent for spot lights
        float m_Attenuation0;      //< constant distance falloff term
        float m_Attenuation1;      //< linear term of falloff
        float m_Attenuation2;      //< quadatic term of falloff

        // NOTE: theta and phi are *half angles*
        float m_Theta; //< inner cone angle. no angular falloff
        //< within this cone
        float m_Phi; //< outer cone angle

        // the values below are derived from the above settings for optimizations
        // These aren't used by DX8. . used for software lighting.

        // NOTE: These dots are cos( m_Theta ), cos( m_Phi )
        float m_ThetaDot;
        float m_PhiDot;
        float m_OneOverThetaDotMinusPhiDot;
        uint32_t m_Flags;

      protected:
        float m_RangeSquared;

#if 0
	public:

		void RecalculateDerivedValues(void);			 // calculate m_xxDot, m_Type for changed parms
		void RecalculateOneOverThetaDotMinusPhiDot( );

		LightDesc_t(void)
		{
		}

		// constructors for various useful subtypes

		// a point light with infinite range
		LightDesc_t(const math::vector3& pos, const math::vector3& color)
		{
			InitPoint(pos, color);
		}

		LightDesc_t& operator=(const LightDesc_t& src)
		{
			memcpy(this, &src, sizeof(LightDesc_t));
			return *this;
		}

		/// a simple light. cone boundaries in radians. you pass a look_at point and the
		/// direciton is derived from that.
		LightDesc_t(const math::vector3& pos, const math::vector3& color, const math::vector3& point_at,
					float inner_cone_boundary, float outer_cone_boundary)
		{
			InitSpot(pos, color, point_at, inner_cone_boundary, outer_cone_boundary);
		}

		void InitPoint(const math::vector3& pos, const math::vector3& color);
		void InitDirectional(const math::vector3& dir, const math::vector3& color);
		void InitSpot(const math::vector3& pos, const math::vector3& color, const math::vector3& point_at,
					  float inner_cone_boundary, float outer_cone_boundary);

		/// Given 4 points and 4 normals, ADD lighting from this light into "color".
		void ComputeLightAtPoints(const FourVectors& pos, const FourVectors& normal,
								  FourVectors& color, bool DoHalfLambert = false) const;
		void ComputeNonincidenceLightAtPoints(const FourVectors& pos, FourVectors& color) const;
		void ComputeLightAtPointsForDirectional(const FourVectors& pos,
												const FourVectors& normal,
												FourVectors& color, bool DoHalfLambert = false) const;

		// warning - modifies color!!! set color first!!
		void SetupOldStyleAttenuation(float fQuadatricAttn, float fLinearAttn, float fConstantAttn);

		void SetupNewStyleAttenuation(float fFiftyPercentDistance, float fZeroPercentDistance);


		/// given a direction relative to the light source position, is this ray within the
			/// light cone (for spotlights..non spots consider all rays to be within their cone)
		bool IsDirectionWithinLightCone(const math::vector3& rdir) const
		{
			return ((m_Type != SPOT) || (rdir.Dot(m_Direction) >= m_PhiDot));
		}

		float OneOverThetaDotMinusPhiDot( ) const
		{
			return m_OneOverThetaDotMinusPhiDot;
		}

		float DistanceAtWhichBrightnessIsLessThan(float flAmount) const;

#endif
    };

    struct MaterialLightingState_t
    {
        math::vector3 vecAmbientCube[6];
        math::vector3 vecLightingOrigin;
        int nLocalLightCount;
        LightDesc_t localLightDesc[4];
    };

    struct FlashlightState_t;
    struct DrawModelResults_t;
    struct ColorMeshInfo_t;

    struct DrawModelInfo_t
    {
        studiohdr_t* pStudioHdr;
        studiohwdata_t* pHardwareData;
        StudioDecalHandle_t hDecals;
        int iSkin;
        int iBody;
        int iHitboxSet;
        client_renderable* pClientEntity;
        int iLOD;
        ColorMeshInfo_t* pColorMeshes;
        bool StaticLighting;
        MaterialLightingState_t lightingState;
    };

    struct UberlightState_t
    {
        UberlightState_t()
        {
            m_fNearEdge  = 2.0f;
            m_fFarEdge   = 100.0f;
            m_fCutOn     = 10.0f;
            m_fCutOff    = 650.0f;
            m_fShearx    = 0.0f;
            m_fSheary    = 0.0f;
            m_fWidth     = 0.3f;
            m_fWedge     = 0.05f;
            m_fHeight    = 0.3f;
            m_fHedge     = 0.05f;
            m_fRoundness = 0.8f;
        }

        float m_fNearEdge;
        float m_fFarEdge;
        float m_fCutOn;
        float m_fCutOff;
        float m_fShearx;
        float m_fSheary;
        float m_fWidth;
        float m_fWedge;
        float m_fHeight;
        float m_fHedge;
        float m_fRoundness;
    };

    struct FlashlightState_t
    {
        FlashlightState_t()
        {
            std::memset(this, 0, sizeof(FlashlightState_t));
            EnableShadows             = false; // Provide reasonable defaults for shadow depth mapping parameters
            DrawShadowFrustum         = false;
            ShadowMapResolution       = 1024.0f;
            ShadowFilterSize          = 3.0f;
            ShadowSlopeScaleDepthBias = 16.0f;
            ShadowDepthBias           = 0.0005f;
            ShadowJitterSeed          = 0.0f;
            FlashlightTime            = 0.0f;
            ShadowAtten               = 0.0f;
            AmbientOcclusion          = 0.0f;
            Scissor                   = false;
            Left                      = -1;
            Top                       = -1;
            Right                     = -1;
            Bottom                    = -1;
            ShadowQuality             = 0;
            ShadowHighRes             = false;
            Uberlight                 = false;
            Volumetric                = false;
            NoiseStrength             = 0.8f;
            NumPlanes                 = 64;
            PlaneOffset               = 0.0f;
            VolumetricIntensity       = 1.0f;
            Ortho                     = false;
            m_fOrthoLeft              = -1.0f;
            m_fOrthoRight             = 1.0f;
            m_fOrthoTop               = -1.0f;
            m_fOrthoBottom            = 1.0f;
            ProjectionSize            = 500.0f;
            ProjectionRotation        = 0.0f;
        }

        math::vector3 m_vecLightOrigin;
        math::quaternion m_quatOrientation;
        float m_NearZ;
        float m_FarZ;
        float m_fHorizontalFOVDegrees;
        float m_fVerticalFOVDegrees;
        bool Ortho;
        float m_fOrthoLeft;
        float m_fOrthoRight;
        float m_fOrthoTop;
        float m_fOrthoBottom;
        float m_fQuadraticAtten;
        float m_fLinearAtten;
        float m_fConstantAtten;
        float m_FarZAtten;
        float m_Color[4];
        ITexture* SpotlightTexture;
        material* ProjectedMaterial;
        int SpotlightTextureFrame;

        // Shadow depth mapping parameters
        bool EnableShadows;
        bool DrawShadowFrustum;
        float ShadowMapResolution;
        float ShadowFilterSize;
        float ShadowSlopeScaleDepthBias;
        float ShadowDepthBias;
        float ShadowJitterSeed;
        float ShadowAtten;
        float AmbientOcclusion;
        int ShadowQuality;
        bool ShadowHighRes;

        // simple projection
        float ProjectionSize;
        float ProjectionRotation;

        // Uberlight parameters
        bool Uberlight;
        UberlightState_t m_uberlightState;

        bool Volumetric;
        float NoiseStrength;
        float FlashlightTime;
        int NumPlanes;
        float PlaneOffset;
        float VolumetricIntensity;

#if 0
		// Getters for scissor members
		bool DoScissor( ) const { return Scissor; }
		int GetLeft( ) const { return Left; }
		int GetTop( ) const { return Top; }
		int GetRight( ) const { return Right; }
		int GetBottom( ) const { return Bottom; }

	private:
#endif
        bool Scissor;
        int Left;
        int Top;
        int Right;
        int Bottom;
    };

    struct FlashlightInstance_t
    {
        material* DebugMaterial;
        FlashlightState_t m_FlashlightState;
        math::view_matrix m_WorldToTexture;
        ITexture* FlashlightDepthTexture;
    };

    struct StudioModelArrayInfo2_t
    {
        int FlashlightCount;
        FlashlightInstance_t* Flashlights; // NOTE: Can have at most MAX_FLASHLIGHTS_PER_INSTANCE_DRAW_CALL of these
    };

    struct StudioModelArrayInfo_t : public StudioModelArrayInfo2_t
    {
        studiohdr_t* StudioHdr;
        studiohwdata_t* HardwareData;
    };

    struct StudioRenderConfig_t
    {
        float fEyeShiftX; // eye X position
        float fEyeShiftY; // eye Y position
        float fEyeShiftZ; // eye Z position
        float fEyeSize;   // adjustment to iris textures
        float fEyeGlintPixelWidthLODThreshold;

        int maxDecalsPerModel;
        int drawEntities;
        int skin;
        int fullbright;

        bool EyeMove                : 1; // look around
        bool SoftwareSkin           : 1;
        bool NoHardware             : 1;
        bool NoSoftware             : 1;
        bool Teeth                  : 1;
        bool Eyes                   : 1;
        bool Flex                   : 1;
        bool Wireframe              : 1;
        bool DrawNormals            : 1;
        bool DrawTangentFrame       : 1;
        bool DrawZBufferedWireframe : 1;
        bool SoftwareLighting       : 1;
        bool ShowEnvCubemapOnly     : 1;
        bool WireframeDecals        : 1;

        // Reserved for future use
        int Reserved[4];
    };

    struct GetTriangles_Vertex_t
    {
        math::vector3 m_Position;
        math::vector3 m_Normal;
        math::vector4 m_TangentS;
        math::vector2 m_TexCoord;
        math::vector4 m_BoneWeight;
        int m_BoneIndex[4];
        int m_NumBones;
    };

    struct GetTriangles_MaterialBatch_t
    {
        material* Material;
        valve_vector<GetTriangles_Vertex_t> m_Verts;
        valve_vector<int> m_TriListIndices;
    };

    struct GetTriangles_Output_t
    {
        valve_vector<GetTriangles_MaterialBatch_t> m_MaterialBatches;
        math::matrix3x4 m_PoseToWorld[MAXSTUDIOBONES];
    };

    struct StudioShadowArrayInstanceData_t
    {
        int LOD;
        int Body;
        int Skin;
        math::matrix3x4_aligned* PoseToWorld;
        float* FlexWeights;
        float* DelayedFlexWeights;
    };

    struct StudioArrayInstanceData_t : public StudioShadowArrayInstanceData_t
    {
        MaterialLightingState_t* LightingState;
        MaterialLightingState_t* DecalLightingState;
        ITexture* EnvCubemapTexture;
        StudioDecalHandle_t m_Decals;
        uint32_t FlashlightUsage; // Mask indicating which flashlights to use.
        ShaderStencilState_t* StencilState;
        ColorMeshInfo_t* ColorMeshInfo;
        bool ColorMeshHasIndirectLightingOnly;
        math::vector4 m_DiffuseModulation;
    };

    struct StudioArrayData_t
    {
        studiohdr_t* StudioHdr;
        studiohwdata_t* HardwareData;
        void* InstanceData; // See StudioShadowArrayInstanceData_t or StudioArrayInstanceData_t
        int Count;
    };

    class IStudioRender : public app_system
    {
      public:
        /*void SetColorModulation(float const* arrColor)
        {
            MEM::CallVFunc<void>(this, 27, arrColor);
        }

        void SetAlphaModulation(float flAlpha)
        {
            MEM::CallVFunc<void>(this, 28, flAlpha);
        }

        void ForcedMaterialOverride(material* pMaterial, EOverrideType nOverrideType = OVERRIDE_NORMAL, int nOverrides = 0)
        {
            MEM::CallVFunc<void>(this, 33, pMaterial, nOverrideType, nOverrides);
        }*/

        virtual void BeginFrame(void) = 0;
        virtual void EndFrame(void)   = 0;

        // Used for the mat_stub console command.
        virtual void Mat_Stub(material_system* pMatSys) = 0;

        // Updates the rendering configuration
        virtual void UpdateConfig(const StudioRenderConfig_t& config) = 0;
        virtual void GetCurrentConfig(StudioRenderConfig_t& config)   = 0;

        // Load, unload model data
        virtual bool LoadModel(studiohdr_t* pStudioHdr, void* pVtxData, studiohwdata_t* pHardwareData) = 0;
        virtual void UnloadModel(studiohwdata_t* pHardwareData)                                        = 0;

        // Refresh the studiohdr since it was lost...
        virtual void RefreshStudioHdr(studiohdr_t* pStudioHdr, studiohwdata_t* pHardwareData) = 0;

        // This is needed to do eyeglint and calculate the correct texcoords for the eyes.
        virtual void SetEyeViewTarget(const studiohdr_t* pStudioHdr, int nBodyIndex, const math::vector3& worldPosition) = 0;

        // Methods related to lighting state
        // NOTE: SetAmbientLightColors assumes that the arraysize is the same as
        // returned from GetNumAmbientLightSamples
        virtual int GetNumAmbientLightSamples()                                     = 0;
        virtual const math::vector3* GetAmbientLightDirections()                    = 0;
        virtual void SetAmbientLightColors(const math::vector4* pAmbientOnlyColors) = 0;
        virtual void SetAmbientLightColors(const math::vector3* pAmbientOnlyColors) = 0;
        virtual void SetLocalLights(int numLights, const LightDesc_t* pLights)      = 0;

        // Sets information about the camera location + orientation
        virtual void SetViewState(const math::vector3& viewOrigin, const math::vector3& viewRight, const math::vector3& viewUp, const math::vector3& viewPlaneNormal) = 0;

        // LOD stuff
        virtual int GetNumLODs(const studiohwdata_t& hardwareData) const                         = 0;
        virtual float GetLODSwitchValue(const studiohwdata_t& hardwareData, int lod) const       = 0;
        virtual void SetLODSwitchValue(studiohwdata_t& hardwareData, int lod, float switchValue) = 0;

        // Sets the color/alpha modulation
        virtual void SetColorModulation(float const* pColor) = 0;
        virtual void SetAlphaModulation(float flAlpha)       = 0;

        // Draws the model
        virtual void DrawModel(DrawModelResults_t* pResults,
                               const DrawModelInfo_t& info,
                               math::matrix3x4* pBoneToWorld,
                               float* pFlexWeights,
                               float* pFlexDelayedWeights,
                               const math::vector3& modelOrigin,
                               DrawModelFlags_t flags = DrawModelFlags_t::DRAW_ENTIRE_MODEL) = 0;

        // Methods related to static prop rendering
        virtual void DrawModelStaticProp(const DrawModelInfo_t& drawInfo, const math::matrix3x4& modelToWorld, DrawModelFlags_t flags = DrawModelFlags_t::DRAW_ENTIRE_MODEL) = 0;
        virtual void DrawStaticPropDecals(const DrawModelInfo_t& drawInfo, const math::matrix3x4& modelToWorld)                                                              = 0;
        virtual void DrawStaticPropShadows(const DrawModelInfo_t& drawInfo, const math::matrix3x4& modelToWorld, int flags)                                                  = 0;

        // Causes a material to be used instead of the materials the model was compiled with
        virtual void ForcedMaterialOverride(material* newMaterial, OverrideType_t nOverrideType = OverrideType_t::NORMAL, int nMaterialIndex = -1) = 0;
        virtual bool IsForcedMaterialOverride()                                                                                                    = 0;

        // Create, destroy list of decals for a particular model
        virtual StudioDecalHandle_t CreateDecalList(studiohwdata_t* pHardwareData) = 0;
        virtual void DestroyDecalList(StudioDecalHandle_t handle)                  = 0;

        // Add decals to a decal list by doing a planar projection along the ray
        // The BoneToWorld matrices must be set before this is called
        virtual void AddDecal(StudioDecalHandle_t handle,
                              studiohdr_t* pStudioHdr,
                              math::matrix3x4* pBoneToWorld,
                              const Ray_t& ray,
                              const math::vector3& decalUp,
                              material* pDecalMaterial,
                              float radius,
                              int body,
                              bool noPokethru           = false,
                              int maxLODToDecal         = /*ADDDECAL_TO_ALL_LODS*/ -1,
                              void* pvProxyUserData     = nullptr,
                              int nAdditionalDecalFlags = 0) = 0;

        // Compute the lighting at a point and normal
        virtual void ComputeLighting(const math::vector3* pAmbient,
                                     int lightCount,
                                     LightDesc_t* pLights,
                                     const math::vector3& pt,
                                     const math::vector3& normal,
                                     math::vector3& lighting) = 0;

        // Compute the lighting at a point, constant directional component is passed
        // as flDirectionalAmount
        virtual void ComputeLightingConstDirectional(const math::vector3* pAmbient,
                                                     int lightCount,
                                                     LightDesc_t* pLights,
                                                     const math::vector3& pt,
                                                     const math::vector3& normal,
                                                     math::vector3& lighting,
                                                     float flDirectionalAmount) = 0;

        // Shadow state (affects the models as they are rendered)
        virtual void AddShadow(material* pMaterial,
                               void* pProxyData,
                               FlashlightState_t* FlashlightState = nullptr,
                               math::view_matrix* pWorldToTexture = nullptr,
                               ITexture* pFlashlightDepthTexture  = nullptr) = 0;
        virtual void ClearAllShadows()                                      = 0;

        // Gets the model LOD; pass in the screen size in pixels of a sphere
        // of radius 1 that has the same origin as the model to get the LOD out...
        virtual int ComputeModelLod(studiohwdata_t* pHardwareData, float unitSphereSize, float* pMetric = nullptr) = 0;

        // Return a number that is usable for budgets, etc.
        // Things that we care about:
        // 1) effective triangle count (factors in batch sizes, state changes, etc)
        // 2) texture memory usage
        // Get Triangles returns the LOD used
        virtual void GetPerfStats(DrawModelResults_t* pResults, const DrawModelInfo_t& info, CUtlBuffer* pSpewBuf = nullptr) const = 0;
        virtual void GetTriangles(const DrawModelInfo_t& info, math::matrix3x4* pBoneToWorld, GetTriangles_Output_t& out)          = 0;

        // Returns materials used by a particular model
        virtual int GetMaterialList(studiohdr_t* pStudioHdr, int count, material** ppMaterials)                                                       = 0;
        virtual int GetMaterialListFromBodyAndSkin(MDLHandle_t studio, int nSkin, int nBody, int nCountOutputMaterials, material** ppOutputMaterials) = 0;

        // no debug modes, just fastest drawing path
        virtual void DrawModelArrayStaticProp(const DrawModelInfo_t& drawInfo, int nInstanceCount, const MeshInstanceData_t* pInstanceData, ColorMeshInfo_t** pColorMeshes) = 0;

        // draw an array of models with the same state
        virtual void DrawModelArray(const StudioModelArrayInfo_t& drawInfo,
                                    int nCount,
                                    StudioArrayInstanceData_t* pInstanceData,
                                    int nInstanceStride,
                                    DrawModelFlags_t flags = DrawModelFlags_t::DRAW_ENTIRE_MODEL) = 0;

        // draw an array of models with the same state
        virtual void DrawModelShadowArray(int nCount, StudioArrayData_t* pShadowData, int nInstanceStride, DrawModelFlags_t flags = DrawModelFlags_t::DRAW_ENTIRE_MODEL) = 0;

        // draw an array of models with the same state
        virtual void DrawModelArray(const StudioModelArrayInfo2_t& info,
                                    int nCount,
                                    StudioArrayData_t* pArrayData,
                                    int nInstanceStride,
                                    DrawModelFlags_t flags = DrawModelFlags_t::DRAW_ENTIRE_MODEL) = 0;

#ifndef _CERT
        struct FacesRenderedInfo_t
        {
            studiohdr_t* pStudioHdr;
            uint32_t nFaceCount;
            uint32_t nRenderCount;
        };

        typedef void (*FaceInfoCallbackFunc_t)(int nTopN, FacesRenderedInfo_t* pRenderedFaceInfo, int nTotalFaces);
        virtual void GatherRenderedFaceInfo(FaceInfoCallbackFunc_t pFunc) = 0;
#endif // !_CERT
    };
} // namespace fd::csgo
