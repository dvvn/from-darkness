module;

#include "cheat/hooks/base_includes.h" 
#include <vector>
#include <array>

module cheat.hooks.other:rta_holder;
import cheat.csgo.awaiter;

using namespace cheat;
using namespace hooks::other;

template<class C, class T, typename ...Args>
static void _Write_tagets(T& holder, Args&&...args)
{
	holder.reserve(sizeof...(Args));
	(new (&holder.emplace_back( )) C(args), ...);
}

void rta_holder::construct( )noexcept
{
	this->deps( ).add<csgo_awaiter>( );
	//something wrong with copy-move constructors
	//here is workarond

	using byte_storage_type = std::vector<std::array<uint8_t, sizeof(rta_proxy)>>;
	static_assert(sizeof(byte_storage_type) == sizeof(storage_type));

	byte_storage_type proxies;
	_Write_tagets<rta_proxy>(proxies, L"client.dll", L"engine.dll", L"server.dll", L"studiorender.dll", L"materialsystem.dll", L"shaderapidx9.dll", L"vstdlib.dll", L"vguimatsurface.dll");
	proxies_ = reinterpret_cast<storage_type&&>(proxies);
}

bool rta_holder::load( )noexcept
{
	auto ex = executor(1);
	for (auto& p : proxies_)
	{
		if (!p.start(ex, sync_start( )))
			return false;
	}
	return true;
}

