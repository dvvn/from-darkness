module;

#include "cheat/console/includes.h"

#include <nstd/rtlib/includes.h>
#include <nstd/mem/block_includes.h>

#include <nstd/unistring.h>

module cheat.csgo.modules;
import cheat.console;
import nstd.rtlib;
import nstd.mem.block;

using namespace cheat::csgo_modules;
using namespace nstd::rtlib;

template <typename E, typename Tr>
static info* _Get_module(const std::basic_string_view<E, Tr>& target_name)
{
	const auto do_find = []<typename T>(const T & str)
	{
		return all_infos::get_ptr( )->find([&](const info& info)-> bool
										   {
											   return std::ranges::equal(info.name( ), str);
										   });
	};

	if (target_name.rfind('.') == target_name.npos)
	{
		const auto str = nstd::unistring<wchar_t>(target_name).append(L".dll");
		return do_find(str);
	}

	return do_find(target_name);
}

static ifcs_entry_type _Interface_entry(info* target_module)
{
	ifcs_entry_type entry;

	auto& exports = target_module->exports( );
	using namespace std::string_view_literals;
	const auto create_fn = exports.at("CreateInterface"sv).addr;
	const auto reg = create_fn.rel32(0x5).add(0x6).deref(2).ptr<CInterfaceRegister>( );

	std::vector<ifcs_entry_type::value_type> temp_entry;
	for (auto r = reg; r != nullptr; r = r->next)
		temp_entry.emplace_back(r->name, r->create_fn);

	const auto contains_duplicate = [&](const std::string_view& new_string, size_t original_size)
	{
		auto detected = false;
		for (auto& raw_string : temp_entry | std::views::keys)
		{
			if (raw_string.size( ) != original_size)
				continue;
			if (!raw_string.starts_with(new_string))
				continue;

			if (!detected)
				detected = true;
			else
				break;
		}
		return detected;
	};
	const auto drop_underline = [&](const std::string_view& str, size_t original_size) -> std::optional<std::string_view>
	{
		if (str.ends_with('_'))
		{
			const auto str2 = str.substr(0, str.size( ) - 1);
			if (!contains_duplicate(str2, original_size))
				return str2;
		}
		return {};
	};
	const auto get_pretty_string = [&](const std::string_view& str) -> std::optional<std::string_view>
	{
#if 0
		const auto remove = std::ranges::distance(str | std::views::reverse | std::views::take_while(std::isdigit));
#else
		size_t remove = 0;
		for (const auto c : str | std::views::reverse)
		{
			if (!std::isdigit(c))
				break;
			++remove;
		}
#endif
		const auto original_size = str.size( );

		if (remove == 0)
			return drop_underline(str, original_size);

		const auto str2 = str.substr(0, str.size( ) - remove);
		if (!contains_duplicate(str2, original_size))
			return drop_underline(str2, original_size).value_or(str2);
		else
			return drop_underline(str, original_size);
	};

	for (const auto [name, fn] : temp_entry)
	{
		const auto name_pretty = get_pretty_string(name);
		entry.emplace(name_pretty.value_or(name), fn);
	}

	return entry;
}

game_module_storage::game_module_storage(const std::string_view& name)
{
	const auto ptr = _Get_module(name);

	info_ptr = ptr;
	interfaces = _Interface_entry(ptr);
}

game_module_storage::~game_module_storage( ) = default;
game_module_storage::game_module_storage(game_module_storage&&) noexcept = default;
game_module_storage& game_module_storage::operator=(game_module_storage&&) noexcept = default;

template<typename T>
struct transform_cast
{
	static constexpr auto cast_fn = []<typename Q>(Q q)
	{
		return static_cast<T>(q);
	};

	template<typename Rng>
	auto operator()(Rng&& rng)const
	{
		return std::views::transform(std::forward<Rng>(rng), cast_fn);
	}

	template<typename Itr>
	auto operator()(Itr begin, Itr end)const
	{
		return std::invoke(*this, std::subrange(begin, end));
	}
};

template<typename T>
static constexpr auto _Transform_cast = transform_cast<T>();

nstd::address game_module_storage::find_signature(const std::string_view & sig)
{
#ifdef _DEBUG
	auto [itr, added] = sigs_tested.emplace(sig);
	if (!added)
		throw std::logic_error("Signature already added!");
#endif

	//no cache inside

	const auto block = info_ptr->mem_block( );
	const auto bytes = nstd::make_signature < nstd::make_signature_tag_convert{} > (sig.begin( ), sig.end( ));
	const auto ret = block.find_block(bytes);

	if (ret.empty( ))
	{
		console::get( ).log(L"{} -> signature \"{}\" not found", info_ptr->name( ), _Transform_cast<wchar_t>(sig));
		return nullptr;
	}

	return ret._Unchecked_begin( );
}

void* game_module_storage::find_vtable(const std::string_view & class_name)
{
	auto& mgr = info_ptr->vtables( );
	const auto vt = mgr.at(class_name);
#ifdef CHEAT_HAVE_CONSOLE
	const auto created = vtables_tested.emplace(class_name).second;
	if (created)
	{
		const std::wstring_view from_name = info_ptr->name( );
		const auto second_module_name = all_infos::get_ptr( )->rfind([&](const info& info) { return info.name( ) == from_name; });
		const auto module_name = second_module_name == info_ptr ? from_name : info_ptr->full_path( );

		console::get( ).log(L"Found \"{}\" vtable in module \"{}\"", _Transform_cast<wchar_t>(class_name), module_name);
	}
#endif
	return vt.addr.ptr<void>( );
}

nstd::address game_module_storage::find_game_interface(const std::string_view & ifc_name)
{
	const auto found = interfaces.find(ifc_name);
	runtime_assert(found != interfaces.end( ));

#ifdef CHEAT_HAVE_CONSOLE
	const auto raw_ifc_name = [&]( )->std::wstring
	{
		const auto orig_ifc_name = found->first;
		const auto orig_ifc_name_end = orig_ifc_name._Unchecked_end( );
		if (*orig_ifc_name_end == '\0')
			return {};
		const auto real_end_offset = std::char_traits<char>::length(orig_ifc_name_end);
		const auto begin = orig_ifc_name._Unchecked_begin( );
		const auto real_end = orig_ifc_name_end + real_end_offset;
		return std::format(L" ({})", _Transform_cast<wchar_t>(begin, real_end));
	};

	console::get( ).log(L"Found interface {}{} in module \"{}\"", _Transform_cast<wchar_t>(ifc_name), raw_ifc_name( ), info_ptr->name( ));
#endif

	return std::invoke(found->second);
}

void game_module_storage::clear_interfaces_cache( )
{
	std::_Destroy_in_place(interfaces);
	std::_Construct_in_place(interfaces);
}

struct modules_database : std::recursive_mutex, std::vector<std::unique_ptr<game_module_storage>>
{
	modules_database( ) = default;
};

static modules_database _Modules_database;

static game_module_storage* _Get_storage_for(const std::string_view & name, size_t index)
{
	const auto estimated_size = index + 1;
	if (_Modules_database.size( ) < estimated_size)
	{
		const auto lock = std::scoped_lock(_Modules_database);
		while (_Modules_database.size( ) != estimated_size)
			_Modules_database.push_back(nullptr);
		return _Get_storage_for(name, index);
	}

	if (!_Modules_database[index])
	{
		const auto my_thread = std::this_thread::get_id( );
		const auto lock = std::scoped_lock(_Modules_database);
		if (my_thread == std::this_thread::get_id( ))
			_Modules_database[index] = std::make_unique<game_module_storage>(name);
	}

	return _Modules_database[index].get( );
}

void cheat::csgo_modules::reset_interfaces_storage( )
{
	const auto lock = std::unique_lock(_Modules_database);
	for (const auto& m : _Modules_database)
	{
		if (m)
			m->clear_interfaces_cache( );
	}
}

game_module_storage* game_module_base::operator->( ) const
{
	return _Get_storage_for(name_, index_);
}
