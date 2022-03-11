#include <Windows.h>
#include <d3d9.h>

#include <future>

import cheat.hooks;

static DWORD WINAPI setup_hooks(LPVOID hModule)
{
	using namespace cheat;
	hooks::init_all( );
	if (hooks::start( ).get( ))
		return TRUE;
	FreeLibraryAndExitThread(static_cast<HMODULE>(hModule), FALSE);
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
		cheat::hooks::stop(true);
		break;
	}
	}

	return TRUE;
}


