#include <fd/logging/init.h>

#include <windows.h>

#include <cstdlib>

// #define _WINDLL

static bool context();

#ifdef _WINDLL
static HANDLE thread;
static DWORD thread_id;

[[noreturn]]
static void exit_fail()
{
    FreeLibraryAndExitThread(nullptr /*WIP*/, EXIT_FAILURE);
}

static DWORD WINAPI context_proxy(LPVOID param)
{
    if (!context())
        exit_fail();
    return TRUE;
}

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        thread = CreateThread(nullptr, 0, context_proxy, handle, 0, &thread_id);
        if (!thread)
            return EXIT_FAILURE;
        break;
    }
#if 0
    case DLL_THREAD_ATTACH: // Do thread-specific initialization.
        break;
    case DLL_THREAD_DETACH: // Do thread-specific cleanup.
        break;
#endif
    case DLL_PROCESS_DETACH:
        if (reserved != nullptr) // do not do cleanup if process termination scenario
        {
            break;
        }

        // Perform any necessary cleanup.
        break;
    }

    return EXIT_SUCCESS;
}
#else
int main(int argc, char *argv[])
{
    return context() ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

static bool context()
{
    using namespace fd;

    [[maybe_unused]] logger_registrar::init_helper logging;

    return true;
}