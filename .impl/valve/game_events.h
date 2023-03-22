#pragma once

#include <cstdint>

namespace fd::valve
{
    // constexpr auto EVENT_DEBUG_ID_SHUTDOWN = 13;

    class bf_write;
    class bf_read;

    struct game_event
    {
        virtual ~game_event()               = default;
        virtual const char* GetName() const = 0;

        virtual bool IsReliable() const                      = 0;
        virtual bool IsLocal() const                         = 0;
        virtual bool IsEmpty(const char* key_name = nullptr) = 0;

        virtual bool GetBool(const char* key_name = nullptr, bool default_value = false)              = 0;
        virtual int GetInt(const char* key_name = nullptr, int default_value = 0)                     = 0;
        virtual uint64_t GetUint64(const char* key_name = nullptr, unsigned long default_value = 0)   = 0;
        virtual float GetFloat(const char* key_name = nullptr, float default_value = 0.0f)            = 0;
        virtual const char* GetString(const char* key_name = nullptr, const char* default_value = "") = 0;
        virtual const wchar_t* GetWString(const char* key_name, const wchar_t* default_value = L"")   = 0;

        virtual void SetBool(const char* key_name, bool value)              = 0;
        virtual void SetInt(const char* key_name, int value)                = 0;
        virtual void SetUint64(const char* key_name, unsigned long value)   = 0;
        virtual void SetFloat(const char* key_name, float value)            = 0;
        virtual void SetString(const char* key_name, const char* value)     = 0;
        virtual void SetWString(const char* key_name, const wchar_t* value) = 0;
    };

    struct game_event_listener2
    {
        virtual ~game_event_listener2() = default;

        virtual void FireGameEvent(game_event* event) = 0;

        virtual int GetEventDebugID()
        {
            return 42;
        }

        int debug_id;
        bool registered_for_events;
    };

    struct game_event_manager2
    {
        virtual ~game_event_manager2()                                                              = default;
        virtual int LoadEventsFromFile(const char* filename)                                        = 0;
        virtual void Reset()                                                                        = 0;
        virtual bool AddListener(game_event_listener2* listener, const char* name, bool ServerSide) = 0;
        virtual bool FindListener(game_event_listener2* listener, const char* name)                 = 0;
        virtual int RemoveListener(game_event_listener2* listener)                                  = 0;
        virtual game_event* CreateEvent(const char* name, bool Force, unsigned int dwUnknown)       = 0;
        virtual bool FireEvent(game_event* event, bool DontBroadcast = false)                       = 0;
        virtual bool FireEventClientSide(game_event* event)                                         = 0;
        virtual game_event* DuplicateEvent(game_event* event)                                       = 0;
        virtual void FreeEvent(game_event* event)                                                   = 0;
        virtual bool SerializeEvent(game_event* event, bf_write* buf)                               = 0;
        virtual game_event* UnserializeEvent(bf_read* buf)                                          = 0;
    };

} // namespace fd::valve
