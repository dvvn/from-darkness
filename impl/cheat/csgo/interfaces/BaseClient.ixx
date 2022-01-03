export module cheat.csgo.interfaces:BaseClient;
export import :AppSystem;

export namespace cheat::csgo
{
	enum ClientFrameStage_t
	{
		FRAME_UNDEFINED = -1,
		FRAME_START,
		FRAME_NET_UPDATE_START,
		FRAME_NET_UPDATE_POSTDATAUPDATE_START,
		FRAME_NET_UPDATE_POSTDATAUPDATE_END,
		FRAME_NET_UPDATE_END,
		FRAME_RENDER_START,
		FRAME_RENDER_END
	};

	// Used by RenderView
	enum RenderViewInfo_t
	{
		RENDERVIEW_UNSPECIFIED = 0,
		RENDERVIEW_DRAWVIEWMODEL = (1 << 0),
		RENDERVIEW_DRAWHUD = (1 << 1),
		RENDERVIEW_SUPPRESSMONITORRENDERING = (1 << 2),
	};

	class CGlobalVarsBase;
	class ClientClass;

	class IBaseClientDLL
	{
	public:
		virtual int          Connect(CreateInterfaceFn appSystemFactory, CGlobalVarsBase* pGlobals) = 0;
		virtual int          Disconnect( ) = 0;
		virtual int          Init(CreateInterfaceFn appSystemFactory, CGlobalVarsBase* pGlobals) = 0;
		virtual void         PostInit( ) = 0;
		virtual void         Shutdown( ) = 0;
		virtual void         LevelInitPreEntity(const char* pMapName) = 0;
		virtual void         LevelInitPostEntity( ) = 0;
		virtual void         LevelShutdown( ) = 0;
		virtual ClientClass* GetAllClasses( ) = 0;

		// Notification that we're moving into another stage during the frame.
		//void FrameStageNotify(ClientFrameStage_t stage);

		// The engine has received the specified user message, this code is used to dispatch the message handler
		bool DispatchUserMessage(int msg_type, int flags, int size, const void* msg);
	};
}
