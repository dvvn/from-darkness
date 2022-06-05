module;

#include <cstdint>

export module fds.csgo.interfaces.AppSystem;

export namespace fds::csgo
{
    typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
    typedef void* (*InstantiateInterfaceFn)();

    enum InitReturnVal_t : int32_t
    {
        INIT_FAILED = 0,
        INIT_OK,
        INIT_LAST_VAL,
    };

    enum AppSystemTier_t : int32_t
    {
        APP_SYSTEM_TIER0 = 0,
        APP_SYSTEM_TIER1,
        APP_SYSTEM_TIER2,
        APP_SYSTEM_TIER3,

        APP_SYSTEM_TIER_OTHER,
    };

    struct AppSystemInfo_t
    {
        const char* m_pModuleName;
        const char* m_pInterfaceName;
    };

    // vtables size=8
    class IAppSystem
    {
      public:
        virtual bool Connect(CreateInterfaceFn factory)                               = 0; // 0
        virtual void Disconnect()                                                     = 0; // 1
        virtual void* QueryInterface(const char* pInterfaceName)                      = 0; // 2
        virtual InitReturnVal_t Init()                                                = 0; // 3
        virtual void Shutdown()                                                       = 0; // 4
        virtual AppSystemInfo_t* GetDependencies()                                    = 0; // 5
        virtual AppSystemTier_t GetTier()                                             = 0; // 6
        virtual void Reconnect(CreateInterfaceFn factory, const char* pInterfaceName) = 0; // 7
        virtual void UnkFunc()                                                        = 0; // 8
    };
} // namespace fds::csgo
