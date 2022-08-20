module;

#include <cstdint>

export module fd.valve.app_system;

export namespace fd::valve
{
    typedef void* (*create_interface_fn)(const char* name, int* return_code);

    // vtables size=8
    struct app_system
    {
        enum init_result : int32_t
        {
            INIT_FAILED = 0,
            INIT_OK,
            INIT_LAST_VAL,
        };

        struct info_type
        {
            const char* module_name;
            const char* interface_name;
        };

        enum timer_type : int32_t
        {
            APP_SYSTEM_TIER0 = 0,
            APP_SYSTEM_TIER1,
            APP_SYSTEM_TIER2,
            APP_SYSTEM_TIER3,

            APP_SYSTEM_TIER_OTHER,
        };

        virtual bool Connect(create_interface_fn factory)                               = 0; // 0
        virtual void Disconnect()                                                       = 0; // 1
        virtual void* QueryInterface(const char* interface_name)                        = 0; // 2
        virtual init_result Init()                                                      = 0; // 3
        virtual void Shutdown()                                                         = 0; // 4
        virtual info_type* GetDependencies()                                            = 0; // 5
        virtual timer_type GetTier()                                                    = 0; // 6
        virtual void Reconnect(create_interface_fn factory, const char* interface_name) = 0; // 7
        virtual void UnkFunc()                                                          = 0; // 8
    };

} // namespace fd::valve
