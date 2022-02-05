module;

#include "cheat/service/basic_includes.h"
#include <vector>
#include <array>

module cheat.hooks.other:rta_holder;
import :rta_blocker;
import cheat.csgo.awaiter;

using namespace cheat;
using namespace hooks::other;

template<class H, class Tpl, size_t ...I>
static void _Write_tagets_impl(H& holder, const Tpl& targets, std::index_sequence<I...> seq)
{
	holder.reserve(seq.size( ));
	(holder.push_back(std::make_unique<rta_blocker<I>>(std::get<I>(targets))), ...);
}

template<class H, class Tpl>
static void _Write_tagets(H& holder, const Tpl& targets)
{
	_Write_tagets_impl(holder, targets, std::make_index_sequence<std::tuple_size_v<Tpl>>( ));
}

void rta_holder::construct( )noexcept
{
	this->deps( ).add<csgo_awaiter>( );

	constexpr auto targets = std::forward_as_tuple(L"client.dll", L"engine.dll", L"server.dll", L"studiorender.dll", L"materialsystem.dll", L"shaderapidx9.dll", L"vstdlib.dll", L"vguimatsurface.dll");
	_Write_tagets(proxies_, targets);
}

bool rta_holder::load( )noexcept
{
	auto ex = executor(1);
	for (auto& p : proxies_)
	{
		if (!p->start(ex, sync_start( )))
			return false;
	}
	return true;
}

