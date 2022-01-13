#include "cheat/service/includes.h"

#include <nstd/runtime_assert.h>
#include <Windows.h>

import cheat.service;

#pragma comment(lib, "Synchronization.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	using cheat::services_loader;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		services_loader::get_ptr( )->load(hModule);
		break;
	case DLL_PROCESS_DETACH:
		services_loader::get_ptr( )->unload( );
		break;
	}

	return TRUE;
}


