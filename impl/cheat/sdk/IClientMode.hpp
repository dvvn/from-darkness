#pragma once
#include "CUserCmd.hpp"
#include "IGameEventmanager.hpp"
#include "IInputSystem.hpp"
#include "ISurface.hpp"

namespace cheat::csgo
{
	class IGameEvent;
	class AudioState_t;
	class C_BasePlayer;
	class IPanel;
	class C_BaseEntity;

	enum class ClearFlags_t
	{
		VIEW_CLEAR_COLOR = 0x1,
		VIEW_CLEAR_DEPTH = 0x2,
		VIEW_CLEAR_FULL_TARGET = 0x4,
		VIEW_NO_DRAW = 0x8,
		VIEW_CLEAR_OBEY_STENCIL = 0x10,
		VIEW_CLEAR_STENCIL = 0x20,
	};

	enum class MotionBlurMode_t
	{
		MOTION_BLUR_DISABLE = 1,
		MOTION_BLUR_GAME = 2,
		MOTION_BLUR_SFM = 3
	};

#if 0

class CViewSetup
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
    utl::Vector    origin;             //0x00B8 
    utl::Vector    angles;             //0x00C4 
    char      pad_0x00D0[0x7C];   //0x00D0

};//Size=0x014C

#endif

	class CViewSetup
	{
	public:
		int         X;
		int         UnscaledX;
		int         Y;
		int         UnscaledY;
		int         Width;
		int         UnscaledWidth;
		int         Height;
		int         UnscaledHeight;
		bool        Ortho;
		float       OrthoLeft;
		float       OrthoTop;
		float       OrthoRight;
		float       OrthoBottom;
		std::byte   pad0[0x7C];
		float       FOV;
		float       ViewModelFOV;
		utl::Vector vecOrigin;
		utl::QAngle angView;
		float       NearZ;
		float       FarZ;
		float       NearViewmodelZ;
		float       FarViewmodelZ;
		float       AspectRatio;
		float       NearBlurDepth;
		float       NearFocusDepth;
		float       FarFocusDepth;
		float       FarBlurDepth;
		float       NearBlurRadius;
		float       FarBlurRadius;
		float       DoFQuality;
		int         nMotionBlurMode;
		float       ShutterTime;
		utl::Vector vecShutterOpenPosition;
		utl::QAngle vecShutterOpenAngles;
		utl::Vector vecShutterClosePosition;
		utl::QAngle vecShutterCloseAngles;
		float       OffCenterTop;
		float       OffCenterBottom;
		float       OffCenterLeft;
		float       OffCenterRight;
		bool OffCenter : 1;
		bool RenderToSubrectOfLargerScreen : 1;
		bool DoBloomAndToneMapping : 1;
		bool DoDepthOfField : 1;
		bool HDRTarget : 1;
		bool DrawWorldNormal : 1;
		bool CullFontFaces : 1;
		bool CacheFullSceneState : 1;
		bool CSMView : 1;
	};

	class IClientMode
	{
	public:
		virtual ~IClientMode( ) =0;

		// Called before the HUD is initialized.
		virtual auto InitViewport( ) -> void =0;

		// One time init when .dll is first loaded.
		virtual auto Init( ) -> void =0;

		// Called when vgui is shutting down.
		virtual auto VGui_Shutdown( ) -> void = 0;

		// One time call when dll is shutting down
		virtual auto Shutdown( ) -> void =0;

		// Called when switching from one IClientMode to another.
		// This can re-layout the view and such.
		// Note that Enable and Disable are called when the DLL initializes and shuts down.
		virtual auto Enable( ) -> void =0;
		virtual auto EnableWithRootPanel(vgui::VPANEL pRoot) -> void =0;

		// Called when it's about to go into another client mode.
		virtual auto Disable( ) -> void =0;

		// Called when initializing or when the view changes.
		// This should move the viewport into the correct position.
		virtual auto Layout(bool bForce = false) -> void =0;

		// Gets at the viewport, if there is one...
		virtual auto GetViewport( ) -> vgui::Panel* = 0;

		// Gets a panel hierarchically below the viewport by name like so "ASWHudInventoryMode/SuitAbilityPanel/ItemPanel1"...
		virtual auto GetPanelFromViewport(const char* pchNamePath) -> vgui::Panel* = 0;

		// Gets at the viewports vgui panel animation controller, if there is one...
		virtual auto GetViewportAnimationController( ) -> vgui::AnimationController* = 0;

		// called every time shared client dll/engine data gets changed,
		// and gives the cdll a chance to modify the data.
		virtual auto ProcessInput(bool bActive) -> void = 0;

		// The mode can choose to draw/not draw entities.
		virtual auto ShouldDrawDetailObjects( ) -> bool = 0;
		virtual auto ShouldDrawEntity(C_BaseEntity* pEnt) -> bool = 0;
		virtual auto ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) -> bool = 0;
		virtual auto ShouldDrawParticles( ) -> bool = 0;

		// The mode can choose to not draw fog
		virtual auto ShouldDrawFog( ) -> bool = 0;

		virtual auto OverrideView(CViewSetup* pSetup) -> void = 0;
		virtual auto OverrideAudioState(AudioState_t* pAudioState) -> void = 0;
		virtual auto KeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding) -> int = 0;
		virtual auto StartMessageMode(int iMessageModeType) -> void = 0;
		virtual auto GetMessagePanel( ) -> vgui::Panel* = 0;
		virtual auto OverrideMouseInput(float* x, float* y) -> void = 0;
		virtual auto CreateMove(float flInputSampleTime, CUserCmd* cmd) -> bool = 0;

		virtual auto LevelInit(const char* newmap) -> void = 0;
		virtual auto LevelShutdown( ) -> void = 0;

		// Certain modes hide the view model
		virtual auto ShouldDrawViewModel( ) -> bool = 0;
		virtual auto ShouldDrawCrosshair( ) -> bool = 0;

		// Let mode override viewport for engine
		virtual auto AdjustEngineViewport(int& x, int& y, int& width, int& height) -> void = 0;

		// Called before rendering a view.
		virtual auto PreRender(CViewSetup* pSetup) -> void = 0;

		// Called after everything is rendered.
		virtual auto PostRender( ) -> void = 0;

		virtual auto PostRenderVGui( ) -> void = 0;

		virtual auto ActivateInGameVGuiContext(vgui::Panel* pPanel) -> void = 0;
		virtual auto DeactivateInGameVGuiContext( ) -> void = 0;
		virtual auto GetViewModelFOV( ) -> float = 0;

		virtual auto CanRecordDemo(char* errorMsg, int length) const -> bool = 0;

		virtual auto GetServerName( ) -> wchar_t* = 0;
		virtual auto SetServerName(wchar_t* name) -> void = 0;
		virtual auto GetMapName( ) -> wchar_t* = 0;
		virtual auto SetMapName(wchar_t* name) -> void = 0;

		virtual auto OnColorCorrectionWeightsReset( ) -> void = 0;
		virtual auto GetColorCorrectionScale( ) const -> float = 0;

		virtual auto HudElementKeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding) -> int = 0;

		virtual auto DoPostScreenSpaceEffects(const CViewSetup* pSetup) -> void = 0;

		virtual auto UpdateCameraManUIState(int iType, int nOptionalParam, uint64_t xuid) -> void = 0;
		virtual auto ScoreboardOff( ) -> void = 0;
		virtual auto GraphPageChanged( ) -> void = 0;

		// Updates.
	public:
		// Called every frame.
		virtual auto Update( ) -> void =0;

		virtual auto SetBlurFade(float scale) -> void = 0;
		virtual auto GetBlurFade( ) -> float = 0;
	};

	class CBaseHudWeaponSelection;
	class CBaseViewport;
	class IUserMessageBinder;
	class CBaseHudChat;

	

	namespace vgui
	{
		typedef unsigned long HCursor;
	}

	class ClientModeShared: public IClientMode, public /*CGameEventListener*/IGameEventListener2
	{
		// IClientMode overrides.
	public:
		#if 0
		virtual ~ClientModeShared( ) =0;


	virtual void	Init();
	virtual void	InitViewport();
	virtual void	VGui_Shutdown();
	virtual void	Shutdown();

	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );

	virtual void	Enable();
	virtual void	EnableWithRootPanel( vgui::VPANEL pRoot );
	virtual void	Disable();
	virtual void	Layout( bool bForce = false );

	virtual void	ReloadScheme( void );
	virtual void	ReloadSchemeWithRoot( vgui::VPANEL pRoot );
	virtual void	OverrideView( CViewSetup *pSetup );
	virtual bool	OverrideRenderBounds( int &x, int &y, int &w, int &h, int &insetX, int &insetY ) { return false; }
	virtual void	OverrideAudioState( AudioState_t *pAudioState ) { return; }
	virtual bool	ShouldDrawDetailObjects( );
	virtual bool	ShouldDrawEntity(C_BaseEntity *pEnt);
	virtual bool	ShouldDrawLocalPlayer( C_BasePlayer *pPlayer );
	virtual bool	ShouldDrawViewModel();
	virtual bool	ShouldDrawParticles( );
	virtual bool	ShouldDrawCrosshair( void );
	virtual void	AdjustEngineViewport( int& x, int& y, int& width, int& height );
	virtual void	PreRender(CViewSetup *pSetup);
	virtual void	PostRender();
	virtual void	PostRenderVGui();
	virtual void	ProcessInput(bool bActive);
	virtual bool	CreateMove( float flInputSampleTime, CUserCmd *cmd );
	virtual void	Update();
	virtual void	SetBlurFade( float scale ) {}
	virtual float	GetBlurFade( void ) { return 0.0f; }

	// Input
	virtual int		KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );
	virtual int		HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );
	virtual void	OverrideMouseInput( float *x, float *y );
	virtual void	StartMessageMode( int iMessageModeType );
	virtual vgui::Panel *GetMessagePanel();

	virtual void	ActivateInGameVGuiContext( vgui::Panel *pPanel );
	virtual void	DeactivateInGameVGuiContext();

	// The mode can choose to not draw fog
	virtual bool	ShouldDrawFog( void );
	
	virtual float	GetViewModelFOV( void );
	virtual vgui::Panel* GetViewport()=0;// { return m_pViewport; }
	virtual vgui::Panel *GetPanelFromViewport( const char *pchNamePath );

	// Gets at the viewports vgui panel animation controller, if there is one...
	virtual vgui::AnimationController *GetViewportAnimationController()=0;	//	{ return m_pViewport->GetAnimationController(); }
	
	virtual void FireGameEvent( IGameEvent *event );

	virtual bool CanRecordDemo( char *errorMsg, int length ) const { return true; }

	virtual int HandleSpectatorKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );

	virtual void InitChatHudElement( void );
	virtual void InitWeaponSelectionHudElement( void );

	virtual wchar_t*	GetServerName( void ) { return 0; }
	virtual void		SetServerName( wchar_t *name ) {}
	virtual wchar_t*	GetMapName( void ) { return 0; }
	virtual void		SetMapName( wchar_t *name ) {}

	virtual void	UpdateCameraManUIState( int iType, int nOptionalParam, uint64_t xuid );
	virtual void	ScoreboardOff( void );
	virtual void	GraphPageChanged( void );
#endif

		IUserMessageBinder* m_UMCMsgVGUIMenu;//CUserMessageBinder
		IUserMessageBinder* m_UMCMsgRumble;//CUserMessageBinder (IUserMessageBinder* wrapper)

		//protected:
		CBaseViewport* m_pViewport;

		//int			GetSplitScreenPlayerSlot() const;

		//private:
		// Message mode handling
		// All modes share a common chat interface
		CBaseHudChat*            m_pChatElement;
		vgui::HCursor            m_CursorNone;
		CBaseHudWeaponSelection* m_pWeaponSelection;
		int                      m_nRootSize[2];
	};
}
