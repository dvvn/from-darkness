module;

#include <Windows.h>

module cheat.hooks.unloader;
import cheat.hooks.loader;
//import cheat.console;

using namespace cheat;

static HMODULE external_handle;

void hooks::set_external_handle(void* const hmodule) noexcept
{
	external_handle = static_cast<HMODULE>(hmodule);
}

[[noreturn]]
static DWORD WINAPI unload_delayed(LPVOID) noexcept
{
	hooks::stop( );
	Sleep(1000);
	FreeLibraryAndExitThread(external_handle, FALSE);
}

void hooks::unload( ) noexcept
{
	if(!external_handle)
	{
		hooks::stop( );
		PostQuitMessage(FALSE);
	}
	else
	{
		CreateThread(NULL, 0, unload_delayed, nullptr, 0, NULL);
	}
}