#pragma once

#ifdef CreateEvent
#undef CreateEvent
#endif

namespace cheat::csgo
{
	constexpr auto EVENT_DEBUG_ID_INIT = 42;
	constexpr auto EVENT_DEBUG_ID_SHUTDOWN = 13;

	class bf_write;
	class bf_read;
	class IGameEvent
	{
	public:
		virtual      ~IGameEvent( ) = 0;
		virtual auto GetName( ) const -> const char* = 0;

		virtual auto IsReliable( ) const -> bool = 0;
		virtual auto IsLocal( ) const -> bool = 0;
		virtual auto IsEmpty(const char* key_name = nullptr) -> bool = 0;

		virtual auto GetBool(const char* key_name = nullptr, bool default_value = false) -> bool = 0;
		virtual auto GetInt(const char* key_name = nullptr, int default_value = 0) -> int = 0;
		virtual auto GetUint64(const char* key_name = nullptr, unsigned long default_value = 0) -> uint64_t = 0;
		virtual auto GetFloat(const char* key_name = nullptr, float default_value = 0.0f) -> float = 0;
		virtual auto GetString(const char* key_name = nullptr, const char* default_value = "") -> const char* = 0;
		virtual auto GetWString(const char* key_name, const wchar_t* default_value = L"") -> const wchar_t* = 0;

		virtual auto SetBool(const char* key_name, bool value) -> void = 0;
		virtual auto SetInt(const char* key_name, int value) -> void = 0;
		virtual auto SetUint64(const char* key_name, unsigned long value) -> void = 0;
		virtual auto SetFloat(const char* key_name, float value) -> void = 0;
		virtual auto SetString(const char* key_name, const char* value) -> void = 0;
		virtual auto SetWString(const char* key_name, const wchar_t* value) -> void = 0;
	};

	class IGameEventListener2
	{
	public:
		virtual ~IGameEventListener2( ) =0;

		virtual auto FireGameEvent(IGameEvent* event) -> void = 0;
		virtual auto GetEventDebugID( ) -> int = 0;

		int debug_id;
		bool registered_for_events;
	};

	class IGameEventManager2
	{
	public:
		virtual      ~IGameEventManager2( ) = 0;
		virtual auto LoadEventsFromFile(const char* filename) -> int = 0;
		virtual auto Reset( ) -> void = 0;
		virtual auto AddListener(IGameEventListener2* listener, const char* name, bool bServerSide) -> bool = 0;
		virtual auto FindListener(IGameEventListener2* listener, const char* name) -> bool = 0;
		virtual auto RemoveListener(IGameEventListener2* listener) -> int = 0;
		virtual auto CreateEvent(const char* name, bool bForce, unsigned int dwUnknown) -> IGameEvent* = 0;
		virtual auto FireEvent(IGameEvent* event, bool bDontBroadcast = false) -> bool = 0;
		virtual auto FireEventClientSide(IGameEvent* event) -> bool = 0;
		virtual auto DuplicateEvent(IGameEvent* event) -> IGameEvent* = 0;
		virtual auto FreeEvent(IGameEvent* event) -> void = 0;
		virtual auto SerializeEvent(IGameEvent* event, bf_write* buf) -> bool = 0;
		virtual auto UnserializeEvent(bf_read* buf) -> IGameEvent* = 0;
	};

	
}
