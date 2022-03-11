module;

#include <nstd/runtime_assert.h>

#include <Windows.h>

module cheat.hooks:unloader;
import :loader;
import cheat.console;

using namespace cheat;

static HMODULE get_current_handle( )
{
	return 0;//todo
}

static DWORD WINAPI unload_impl(LPVOID)
{
	hooks::stop( );
	Sleep(1000);
	console::disable( );//skip messages from destructors
	FreeLibraryAndExitThread(get_current_handle( ), TRUE);
}

void hooks::unload( )
{
	const HANDLE hThread = CreateThread(NULL, 0, unload_impl, NULL, 0, NULL);
	runtime_assert(hThread != nullptr);
	CloseHandle(hThread);
}