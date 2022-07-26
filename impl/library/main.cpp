#include <Windows.h>
#include <d3d9.h>

#include <thread>

import fd.hooks_loader;
import fd.logger;
import fd.system_console;
import fd.application_info;

IDirect3DDevice9* d3dDevice9_ptr = nullptr;

static void _Init(std::stop_token stop)
{
    fd::app_info.construct(hwnd, hmodule);
    fd::logger.append(fd::system_console_writer);
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		const HANDLE hThread = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			setup_hooks, // Thread start address
			hModule,    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id
		if (hThread == NULL)
		{
			// Thread creation failed.
			// More details can be retrieved by calling GetLastError()
			return FALSE;
		}
		CloseHandle(hThread);
		break;
	}
	case DLL_PROCESS_DETACH:
	{
        fd::hooks_loader->disable();
        break;
    }
    }

	return TRUE;
}
