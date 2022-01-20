#include "cheat/service/root_includes.h"

#include <nstd/runtime_assert.h>
#include <Windows.h>

import cheat.service;

#pragma comment(lib, "Synchronization.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

using namespace cheat;

template<typename Holder>
static void register_services(service<Holder>& loader)
{
	auto deps = loader.deps( );
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		auto& loader = services_loader::get( );
		service_deps_getter_add_allow_skip = false;
		loader.module_handle = hModule;
		register_services(loader);
		loader.start_async(services_loader::async_detach( ));
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		services_loader::get( ).reset(true);
		break;
	}
	}

	return TRUE;
}


