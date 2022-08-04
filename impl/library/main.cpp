#include <fd_init.h>

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH: {
        fd::init(nullptr, hModule);
        break;
    }
    case DLL_PROCESS_DETACH: {
        fd::destroy();
        break;
    }
    }

    return TRUE;
}
