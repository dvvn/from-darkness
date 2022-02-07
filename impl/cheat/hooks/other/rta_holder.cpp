module;

#include "cheat/service/basic_includes.h"
#include <dhooks/includes.h>
#include <vector>
#include <array>

module cheat.hooks.other:rta_holder;
import :rta_blocker;
import cheat.csgo.awaiter;

using namespace cheat;
using namespace hooks::other;

template<size_t Idx, class H, class Tpl>
static bool _Emplace(H& holder, const Tpl& targets)
{
	auto item = std::make_unique<rta_blocker<Idx>>( );
	if (!item->load(std::get<Idx>(targets)))
		return false;
	holder.push_back(std::move(item));
	return true;
}

template<class H, class Tpl, size_t ...I>
static bool _Init_tagets_impl(H& holder, const Tpl& targets, std::index_sequence<I...> seq)
{
	holder.reserve(seq.size( ));
	return (_Emplace<I>(holder, targets) && ...);
}

template<class H, class Tpl>
static bool _Init_tagets(H& holder, const Tpl& targets)
{
	return _Init_tagets_impl(holder, targets, std::make_index_sequence<std::tuple_size_v<Tpl>>( ));
}

template<typename ...T>
static constexpr auto _Set_targetes(T&& ...args)
{
	return std::make_tuple(std::basic_string_view(args)...);
}

void rta_holder::construct( )noexcept
{
	this->deps( ).add<csgo_awaiter>( );
}

bool rta_holder::load( )noexcept
{
	constexpr auto targets = _Set_targetes(L"client.dll", L"engine.dll", L"server.dll", L"studiorender.dll", L"materialsystem.dll", L"shaderapidx9.dll", L"vstdlib.dll", L"vguimatsurface.dll");
	return _Init_tagets(proxies_, targets);
}

void rta_holder::request_disable( )
{
	for (auto& p : proxies_)
		p->request_disable( );
}

