//module;
//
//#include "cheat/hooks/base_includes.h"
//#include <nstd/rtlib/includes.h>
//#include <nstd/mem/signature_includes.h>
//#include <nstd/ranges.h>
//#include <string>
//
//module cheat.hooks.other:rta_blocker;
//import cheat.csgo.modules;
//
//using namespace cheat;
//using namespace hooks;
//
//static auto _Get_lib(const nstd::hashed_wstring_view& name)
//{
//	using namespace nstd;
//	return std::ranges::find(rtlib::all_infos::get( ), name, [](const rtlib::info& i)->hashed_wstring_view {return i.name( ).fixed; });
//}
//
//void* other::rta_blocker_get_target_method(std::string_view name, size_t index)
//{
//	auto lib = _Get_lib(module_name);
//	auto block = lib->mem_block( );
//	auto sig = nstd::mem::make_signature("55 8B EC 56 8B F1 33 C0 57 8B 7D 08");
//	auto found = block.find_block(sig);
//	return csgo_modules::client->find_signature("55 8B EC 56 8B F1 33 C0 57 8B 7D 08");
//}