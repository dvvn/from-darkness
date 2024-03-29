module;

#include <cstdint>

export module fd.valve.client_mode;
export import fd.valve.GameEvents;
export import fd.valve.user_cmd;

class AudioState_t;
class base_player;
class IPanel;
class base_entity;

using namespace fd::valve;
using namespace fd::math;

enum class ClearFlags_t
{
    VIEW_CLEAR_COLOR        = 0x1,
    VIEW_CLEAR_DEPTH        = 0x2,
    VIEW_CLEAR_FULL_TARGET  = 0x4,
    VIEW_NO_DRAW            = 0x8,
    VIEW_CLEAR_OBEY_STENCIL = 0x10,
    VIEW_CLEAR_STENCIL      = 0x20,
};

enum class MotionBlurMode_t
{
    MOTION_BLUR_DISABLE = 1,
    MOTION_BLUR_GAME    = 2,
    MOTION_BLUR_SFM     = 3
};

#if 0

	class view_setup
	{
	public:
		__int32   x;                  //0x0000
		__int32   x_old;              //0x0004
		__int32   y;                  //0x0008
		__int32   y_old;              //0x000C
		__int32   width;              //0x0010
		__int32   width_old;          //0x0014
		__int32   height;             //0x0018
		__int32   height_old;         //0x001C
		char      pad_0x0020[0x90];   //0x0020
		float     fov;                //0x00B0
		float     viewmodel_fov;      //0x00B4
		vector3    origin;             //0x00B8
		vector3    angles;             //0x00C4
		char      pad_0x00D0[0x7C];   //0x00D0

	};//Size=0x014C

#endif

struct view_setup
{
    int X;
    int UnscaledX;
    int Y;
    int UnscaledY;
    int Width;
    int UnscaledWidth;
    int Height;
    int UnscaledHeight;
    bool Ortho;
    float OrthoLeft;
    float OrthoTop;
    float OrthoRight;
    float OrthoBottom;
    uint8_t pad0[0x7C];
    float FOV;
    float ViewModelFOV;
    vector3 vecOrigin;
    qangle angView;
    float NearZ;
    float FarZ;
    float NearViewmodelZ;
    float FarViewmodelZ;
    float AspectRatio;
    float NearBlurDepth;
    float NearFocusDepth;
    float FarFocusDepth;
    float FarBlurDepth;
    float NearBlurRadius;
    float FarBlurRadius;
    float DoFQuality;
    int nMotionBlurMode;
    float ShutterTime;
    vector3 vecShutterOpenPosition;
    qangle vecShutterOpenAngles;
    vector3 vecShutterClosePosition;
    qangle vecShutterCloseAngles;
    float OffCenterTop;
    float OffCenterBottom;
    float OffCenterLeft;
    float OffCenterRight;
    bool OffCenter                     : 1;
    bool RenderToSubrectOfLargerScreen : 1;
    bool DoBloomAndToneMapping         : 1;
    bool DoDepthOfField                : 1;
    bool HDRTarget                     : 1;
    bool DrawWorldNormal               : 1;
    bool CullFontFaces                 : 1;
    bool CacheFullSceneState           : 1;
    bool CSMView                       : 1;
};

namespace vgui
{
    typedef unsigned int VPANEL;

    class Panel;
    class AnimationController;
} // namespace vgui

enum ButtonCode_t;

struct client_mode
{
    virtual ~client_mode() = default;

    // Called before the HUD is initialized.
    virtual void InitViewport() = 0;

    // One time init when .dll is first loaded.
    virtual void Init() = 0;

    // Called when vgui is shutting down.
    virtual void VGui_Shutdown() = 0;

    // One time call when dll is shutting down
    virtual void Shutdown() = 0;

    // Called when switching from one client_mode to another.
    // This can re-layout the view and such.
    // Note that Enable and Disable are called when the DLL initializes and shuts down.
    virtual void Enable()                                = 0;
    virtual void EnableWithRootPanel(vgui::VPANEL pRoot) = 0;

    // Called when it's about to go into another client mode.
    virtual void Disable() = 0;

    // Called when initializing or when the view changes.
    // This should move the viewport into the correct position.
    virtual void Layout(bool Force = false) = 0;

    // Gets at the viewport, if there is one...
    virtual vgui::Panel* GetViewport() = 0;

    // Gets a panel hierarchically below the viewport by name like so "ASWHudInventoryMode/SuitAbilityPanel/ItemPanel1"...
    virtual vgui::Panel* GetPanelFromViewport(const char* pchNamePath) = 0;

    // Gets at the viewports vgui panel animation_ controller, if there is one...
    virtual vgui::AnimationController* GetViewportAnimationController() = 0;

    // called every time shared client dll/engine data gets changed,
    // and gives the cdll a chance to modify the data.
    virtual void ProcessInput(bool Active) = 0;

    // The mode can choose to draw/not draw entities.
    virtual bool ShouldDrawDetailObjects()                   = 0;
    virtual bool ShouldDrawEntity(base_entity* pEnt)         = 0;
    virtual bool ShouldDrawLocalPlayer(base_player* pPlayer) = 0;
    virtual bool ShouldDrawParticles()                       = 0;

    // The mode can choose to not draw fog
    virtual bool ShouldDrawFog() = 0;

    virtual void OverrideView(view_setup* pSetup)                                      = 0;
    virtual void OverrideAudioState(AudioState_t* pAudioState)                         = 0;
    virtual int KeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding) = 0;
    virtual void StartMessageMode(int iMessageModeType)                                = 0;
    virtual vgui::Panel* GetMessagePanel()                                             = 0;
    virtual void OverrideMouseInput(float* x, float* y)                                = 0;
    virtual bool CreateMove(float flInputSampleTime, user_cmd* cmd)                    = 0;

    virtual void LevelInit(const char* newmap) = 0;
    virtual void LevelShutdown()               = 0;

    // Certain modes hide the view model
    virtual bool ShouldDrawViewModel() = 0;
    virtual bool ShouldDrawCrosshair() = 0;

    // Let mode override viewport for engine
    virtual void AdjustEngineViewport(int& x, int& y, int& width, int& height) = 0;

    // Called before rendering a view.
    virtual void PreRender(view_setup* pSetup) = 0;

    // Called after everything is rendered.
    virtual void PostRender() = 0;

    virtual void PostRenderVGui() = 0;

    virtual void ActivateInGameVGuiContext(vgui::Panel* pPanel) = 0;
    virtual void DeactivateInGameVGuiContext()                  = 0;
    virtual float GetViewModelFOV()                             = 0;

    virtual bool CanRecordDemo(char* errorMsg, int length) const = 0;

    virtual wchar_t* GetServerName()          = 0;
    virtual void SetServerName(wchar_t* name) = 0;
    virtual wchar_t* GetMapName()             = 0;
    virtual void SetMapName(wchar_t* name)    = 0;

    virtual void OnColorCorrectionWeightsReset()  = 0;
    virtual float GetColorCorrectionScale() const = 0;

    virtual int HudElementKeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding) = 0;

    virtual void DoPostScreenSpaceEffects(const view_setup* pSetup) = 0;

    virtual void UpdateCameraManUIState(int iType, int nOptionalParam, uint64_t xuid) = 0;
    virtual void ScoreboardOff()                                                      = 0;
    virtual void GraphPageChanged()                                                   = 0;

    // Updates.
  public:
    // Called every frame.
    virtual void Update() = 0;

    virtual void SetBlurFade(float scale) = 0;
    virtual float GetBlurFade()           = 0;
};

class CBaseHudWeaponSelection;
class CBaseViewport;
class IUserMessageBinder;
class CBaseHudChat;

namespace vgui
{
    typedef unsigned long HCursor;
}

struct client_mode_shared : client_mode, /*CGameEventListener*/ game_event_listener2
{
    // client_mode overrides.
#if 0
		virtual ~client_mode_shared( ) = 0;


		virtual void	Init( );
		virtual void	InitViewport( );
		virtual void	VGui_Shutdown( );
		virtual void	Shutdown( );

		virtual void	LevelInit(const char* newmap);
		virtual void	LevelShutdown(void);

		virtual void	Enable( );
		virtual void	EnableWithRootPanel(vgui::VPANEL pRoot);
		virtual void	Disable( );
		virtual void	Layout(bool Force = false);

		virtual void	ReloadScheme(void);
		virtual void	ReloadSchemeWithRoot(vgui::VPANEL pRoot);
		virtual void	OverrideView(view_setup* pSetup);
		virtual bool	OverrideRenderBounds(int& x, int& y, int& w, int& h, int& insetX, int& insetY) { return false; }
		virtual void	OverrideAudioState(AudioState_t* pAudioState) { return; }
		virtual bool	ShouldDrawDetailObjects( );
		virtual bool	ShouldDrawEntity(base_entity* pEnt);
		virtual bool	ShouldDrawLocalPlayer(base_player* pPlayer);
		virtual bool	ShouldDrawViewModel( );
		virtual bool	ShouldDrawParticles( );
		virtual bool	ShouldDrawCrosshair(void);
		virtual void	AdjustEngineViewport(int& x, int& y, int& width, int& height);
		virtual void	PreRender(view_setup* pSetup);
		virtual void	PostRender( );
		virtual void	PostRenderVGui( );
		virtual void	ProcessInput(bool Active);
		virtual bool	CreateMove(float flInputSampleTime, user_cmd* cmd);
		virtual void	Update( );
		virtual void	SetBlurFade(float scale) { }
		virtual float	GetBlurFade(void) { return 0.0f; }

		// Input
		virtual int		KeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding);
		virtual int		HudElementKeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding);
		virtual void	OverrideMouseInput(float* x, float* y);
		virtual void	StartMessageMode(int iMessageModeType);
		virtual vgui::Panel* GetMessagePanel( );

		virtual void	ActivateInGameVGuiContext(vgui::Panel* pPanel);
		virtual void	DeactivateInGameVGuiContext( );

		// The mode can choose to not draw fog
		virtual bool	ShouldDrawFog(void);

		virtual float	GetViewModelFOV(void);
		virtual vgui::Panel* GetViewport( ) = 0;// { return Viewport; }
		virtual vgui::Panel* GetPanelFromViewport(const char* pchNamePath);

		// Gets at the viewports vgui panel animation_ controller, if there is one...
		virtual vgui::AnimationController* GetViewportAnimationController( ) = 0;	//	{ return Viewport->GetAnimationController(); }

		virtual void FireGameEvent(game_event* event);

		virtual bool CanRecordDemo(char* errorMsg, int length) const { return true; }

		virtual int HandleSpectatorKeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding);

		virtual void InitChatHudElement(void);
		virtual void InitWeaponSelectionHudElement(void);

		virtual wchar_t* GetServerName(void) { return 0; }
		virtual void		SetServerName(wchar_t* name) { }
		virtual wchar_t* GetMapName(void) { return 0; }
		virtual void		SetMapName(wchar_t* name) { }

		virtual void	UpdateCameraManUIState(int iType, int nOptionalParam, uint64_t xuid);
		virtual void	ScoreboardOff(void);
		virtual void	GraphPageChanged(void);


    IUserMessageBinder* m_UMCMsgVGUIMenu; // CUserMessageBinder
    IUserMessageBinder* m_UMCMsgRumble;   // CUserMessageBinder (IUserMessageBinder* wrapper)

    // protected:
    CBaseViewport* Viewport;

    // int			GetSplitScreenPlayerSlot() const;

    // private:
    //  Message mode handling
    //  All modes share a common chat interface
    CBaseHudChat* ChatElement;
    vgui::HCursor m_CursorNone;
    CBaseHudWeaponSelection* WeaponSelection;
    int RootSize[2];
#endif
};

export namespace fd::valve
{
    using ::client_mode;
    using ::client_mode_shared;
} // namespace fd::valve
