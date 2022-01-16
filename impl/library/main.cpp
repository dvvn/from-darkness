#include "cheat/service/includes.h"

#include <nstd/runtime_assert.h>
#include <Windows.h>

import cheat.service;

#pragma comment(lib, "Synchronization.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	using srv_loader = cheat::services_loader;
	auto& loader = srv_loader::get( );

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		loader.module_handle = hModule;
		loader.load_async(std::make_unique<srv_loader::executor>( ));
		break;
	case DLL_PROCESS_DETACH:
		loader.reset( );
		break;
	}

	return TRUE;
}


