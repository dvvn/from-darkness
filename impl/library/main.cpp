#include "cheat/service/includes.h"

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
	service_deps_getter_add_allow_skip = false;
	auto deps = loader.deps( );
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	auto& loader = services_loader::get( );

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		loader.module_handle = hModule;
		register_services(loader);
		loader.load_async(std::make_unique<basic_service::executor>( ));
		break;
	case DLL_PROCESS_DETACH:
		loader.reset( );
		break;
	}

	return TRUE;
}


