#pragma once
#include "ClientClass.hpp"
#include "GlobalVars.hpp"
#include "IAppSystem.hpp"

namespace cheat::csgo
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

	class IBaseClientDLL
	{
	public:
		virtual auto Connect(CreateInterfaceFn appSystemFactory, CGlobalVarsBase* pGlobals) -> int = 0;
		virtual auto Disconnect( ) -> int = 0;
		virtual auto Init(CreateInterfaceFn appSystemFactory, CGlobalVarsBase* pGlobals) -> int = 0;
		virtual auto PostInit( ) -> void = 0;
		virtual auto Shutdown( ) -> void = 0;
		virtual auto LevelInitPreEntity(const char* pMapName) -> void = 0;
		virtual auto LevelInitPostEntity( ) -> void = 0;
		virtual auto LevelShutdown( ) -> void = 0;
		virtual auto GetAllClasses( ) -> ClientClass* = 0;

		auto DispatchUserMessage(int messageType, int arg, int arg1, void* data) -> bool
		{
			//using DispatchUserMessage_t = bool* (__thiscall*)(void*, int, int, int, void*);
			//return CallVFunction<DispatchUserMessage_t>(this, 38)(this, messageType, arg, arg1, data);
			BOOST_ASSERT(0);
			return 0;
		}
	};
}
