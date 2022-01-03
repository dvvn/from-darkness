module;

#include <cstdint>

export module cheat.csgo.interfaces:GameEvents;

//#ifdef CreateEvent
//#undef CreateEvent
//#endif

export namespace cheat::csgo
{
	constexpr auto EVENT_DEBUG_ID_INIT = 42;
	constexpr auto EVENT_DEBUG_ID_SHUTDOWN = 13;

	class bf_write;
	class bf_read;
	class IGameEvent
	{
	public:
		virtual             ~IGameEvent( ) = default;
		virtual const char* GetName( ) const = 0;

		virtual bool IsReliable( ) const = 0;
		virtual bool IsLocal( ) const = 0;
		virtual bool IsEmpty(const char* key_name = nullptr) = 0;

		virtual bool           GetBool(const char* key_name = nullptr, bool default_value = false) = 0;
		virtual int            GetInt(const char* key_name = nullptr, int default_value = 0) = 0;
		virtual uint64_t       GetUint64(const char* key_name = nullptr, unsigned long default_value = 0) = 0;
		virtual float          GetFloat(const char* key_name = nullptr, float default_value = 0.0f) = 0;
		virtual const char* GetString(const char* key_name = nullptr, const char* default_value = "") = 0;
		virtual const wchar_t* GetWString(const char* key_name, const wchar_t* default_value = L"") = 0;

		virtual void SetBool(const char* key_name, bool value) = 0;
		virtual void SetInt(const char* key_name, int value) = 0;
		virtual void SetUint64(const char* key_name, unsigned long value) = 0;
		virtual void SetFloat(const char* key_name, float value) = 0;
		virtual void SetString(const char* key_name, const char* value) = 0;
		virtual void SetWString(const char* key_name, const wchar_t* value) = 0;
	};

	class IGameEventListener2
	{
	public:
		virtual ~IGameEventListener2( ) = default;

		virtual void FireGameEvent(IGameEvent* event) = 0;
		virtual int  GetEventDebugID( ) = 0;

		int debug_id;
		bool registered_for_events;
	};

	class IGameEventManager2
	{
	public:
		virtual             ~IGameEventManager2( ) = default;
		virtual int         LoadEventsFromFile(const char* filename) = 0;
		virtual void        Reset( ) = 0;
		virtual bool        AddListener(IGameEventListener2* listener, const char* name, bool bServerSide) = 0;
		virtual bool        FindListener(IGameEventListener2* listener, const char* name) = 0;
		virtual int         RemoveListener(IGameEventListener2* listener) = 0;
		virtual IGameEvent* CreateEvent(const char* name, bool bForce, unsigned int dwUnknown) = 0;
		virtual bool        FireEvent(IGameEvent* event, bool bDontBroadcast = false) = 0;
		virtual bool        FireEventClientSide(IGameEvent* event) = 0;
		virtual IGameEvent* DuplicateEvent(IGameEvent* event) = 0;
		virtual void        FreeEvent(IGameEvent* event) = 0;
		virtual bool        SerializeEvent(IGameEvent* event, bf_write* buf) = 0;
		virtual IGameEvent* UnserializeEvent(bf_read* buf) = 0;
	};
}
