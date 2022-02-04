module;

#include "cheat/hooks/base_includes.h"
#include <nstd/rtlib/includes.h>
#include <nstd/mem/signature_includes.h>
#include <string>

module cheat.hooks.other:rta_proxy;
import nstd.mem;
import nstd.rtlib;

using namespace cheat;
using namespace hooks::other;

rta_proxy::rta_proxy(const std::wstring_view& module_name)
	:module_name_(module_name)
{
}

void rta_proxy::construct( ) noexcept
{
}

static auto _Get_lib(const nstd::hashed_wstring_view& name)
{
	using namespace nstd;
	return std::ranges::find(rtlib::all_infos::get( ), name, [](const rtlib::info& i)->nstd::hashed_wstring_view {return i.name( ).fixed; });
}

void* rta_proxy::get_target_method( ) const
{
	auto lib = _Get_lib(module_name_);
	auto block = lib->mem_block( );
	auto sig = nstd::mem::make_signature("55 8B EC 56 8B F1 33 C0 57 8B 7D 08");
	auto found = block.find_block(sig);
	return found.data( );
}

void rta_proxy::callback([[maybe_unused]] const char* module_name)
{
	this->store_return_value(1);
}

