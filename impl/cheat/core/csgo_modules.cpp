module;

#include <nstd/signature.h>
#include <nstd/unistring.h>
#include <nstd/module/all_infos.h>
#include <nstd/custom_types.h>

#include NSTD_UNORDERED_MAP_INCLUDE
#include NSTD_UNORDERED_SET_INCLUDE

#include <ranges>
#include <format>
#include <optional>
#include <sstream>
#include <algorithm>
#include <functional>
#include <mutex>

module cheat.core.csgo_modules;
import cheat.core.console;

using namespace cheat::csgo_modules;

template <typename E, typename Tr>
static info* _Get_module(const std::basic_string_view<E, Tr>& target_name)
{
	const auto do_find = []<typename T>(const T & str)
	{
		return nstd::module::all_infos::get_ptr( )->find([&](const info& info)-> bool
														 {
															 return std::ranges::equal(info.name( ), str);
														 });
	};

	if (target_name.rfind(E('.')) == target_name.npos)
	{
		using ustring = nstd::unistring<wchar_t>;

		constexpr ustring::value_type dot_dll_arr[] = {'.', 'd', 'l', 'l'};
		constexpr std::basic_string_view dot_dll = {dot_dll_arr, 4};

		const auto str = ustring(target_name).append(dot_dll);
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

	std::vector<std::pair<std::string_view, InstantiateInterfaceFn>> temp_entry;
	for (auto r = reg; r != nullptr; r = r->next)
		temp_entry.emplace_back(make_pair(std::string_view(r->name), r->create_fn));

	const auto contains_duplicate = [&](const std::string_view& new_string, size_t original_size)
	{
		auto detected = false;
		for (auto& raw_string : temp_entry | std::views::keys)
		{
			if (raw_string.size( ) != original_size)
				continue;
			if (!raw_string.starts_with(new_string))
				continue;
			if (detected)
				return true;
			detected = true;
		}
		return false;
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
		size_t remove = 0;
		for (const auto c : str | std::views::reverse)
		{
			if (!std::isdigit(c))
				break;

			++remove;
		}

		const auto original_size = str.size( );

		if (remove == 0)
			return drop_underline(str, original_size);

		auto str2 = str;
		str2.remove_suffix(remove);
		if (contains_duplicate(str2, original_size))
			return drop_underline(str, original_size);
		return drop_underline(str2, original_size).value_or(str2);
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

nstd::address game_module_storage::find_signature(const std::string_view & sig)
{
#ifdef _DEBUG
	auto [itr, added] = sigs_tested.emplace(sig);
	if (!added)
		throw;
#endif

	//no cache inside

	const auto block = info_ptr->mem_block( );
	const auto bytes = nstd::make_signature(sig.begin( ), sig.end( ));
	const auto ret = block.find_block(bytes);

	if (ret.empty( ))
	{
		CHEAT_CONSOLE_LOG(std::format(L"{} -> signature \"{}\" not found", info_ptr->name( ), std::wstring(sig.begin( ), sig.end( ))));
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
		const auto found_msg = [&]
		{
			const auto& from_name = info_ptr->name( );
			const auto second_module_name = nstd::module::all_infos::get_ptr( )->rfind([&](const info& info)
																					   {
																						   return info.name( ) == from_name;
																					   });
			const auto module_name = second_module_name == info_ptr ? from_name : info_ptr->full_path( );
			return std::wostringstream( )
				<< "Found \"" << std::wstring(class_name.begin( ), class_name.end( ))
				<< "\" vtable in module \"" << module_name << '\"';
		};
		console::get( ).log(found_msg( ));
	}
#endif
	return vt.addr.ptr<void>( );
}

nstd::address game_module_storage::find_game_interface(const std::string_view & ifc_name)
{
	const auto found = interfaces.find(ifc_name);
	runtime_assert(found != interfaces.end( ));

#ifdef CHEAT_HAVE_CONSOLE
	const auto debug_message = [&]
	{
		const auto original_interface_name = found->first;
		const auto original_interface_name_end = original_interface_name._Unchecked_end( );

		auto msg = std::wostringstream( );
		msg << "Found interface: " << std::wstring(ifc_name.begin( ), ifc_name.end( )) << ' ';
		if (*original_interface_name_end != '\0')
			msg << '(' << std::wstring(original_interface_name.begin( ), original_interface_name.end( )) << original_interface_name_end << ") ";
		msg << "in module " << info_ptr->name( );
		return msg;
	};
	console::get( ).log(debug_message( ));
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
	if (_Modules_database.size( ) < index + 1)
	{
		const auto lock = std::unique_lock(_Modules_database);
		while (_Modules_database.size( ) != index + 1)
			_Modules_database.push_back(nullptr);
		return _Get_storage_for(name, index);
	}

	if (!_Modules_database[index])
	{
		const auto my_thread = std::this_thread::get_id( );
		const auto lock = std::unique_lock(_Modules_database);
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
