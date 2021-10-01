#include "csgo modules.h"
#include "console.h"

#include "cheat/sdk/IAppSystem.hpp"

#include <nstd/signature.h>
#include <nstd/os/module info.h>

#include <robin_hood.h>

#include <format>
#include <sstream>

using nstd::os::module_info;
using cheat::csgo::InstantiateInterfaceFn;
using namespace cheat::csgo_modules;

template <typename E, typename Tr>
static module_info* _Get_module(const std::basic_string_view<E, Tr>& target_name)
{
	constexpr E dot_dll_arr[] = {'.', 'd', 'l', 'l', '\0'};
	constexpr auto dot_dll    = std::basic_string_view<E, Tr>(dot_dll_arr);

	auto target_name_correct = [&]
	{
		if constexpr (std::same_as<E, wchar_t>)
			return target_name;
		else
			return std::wstring(target_name.begin( ), target_name.end( )).append(dot_dll.begin( ), dot_dll.end( ));
	}( );

	auto compare = [&](const std::wstring& full_name)
	{
		if constexpr (std::same_as<E, wchar_t>)
		{
			return full_name.size( ) == target_name.size( ) + dot_dll.size( )
				   && full_name.starts_with(target_name_correct)
				   && full_name.ends_with(dot_dll);
		}
		else
			return full_name == target_name_correct;
	};

	return nstd::os::all_modules::get_ptr( )->find([&](const module_info& info)-> bool
	{
		return compare(info.name( )) == true;
	});
}

module_info* detail::get_module_impl(const std::string_view& target_name)
{
	return _Get_module(target_name);
}

nstd::address detail::find_signature_impl(module_info* md, const std::string_view& sig)
{
	auto bytes = nstd::make_signature(sig);
	auto ret   = md->mem_block( ).find_block(bytes);
	if (!ret.has_value( ))
	{
		CHEAT_CONSOLE_LOG(std::format(L"{} -> signature {} not found", md->name( ), std::wstring(sig.begin( ), sig.end( ))));
		return nullptr;
	}

	return ret->addr( );
}

// ReSharper disable once CppInconsistentNaming
class CInterfaceRegister
{
public:
	InstantiateInterfaceFn create_fn;
	const char* name;
	CInterfaceRegister* next;
};

using ifcs_entry_type = robin_hood::unordered_map<std::string_view, InstantiateInterfaceFn>;
using ifcs_storage_type = robin_hood::unordered_map<module_info*, ifcs_entry_type>;

static const ifcs_entry_type& _Interface_entry(ifcs_storage_type& storage, module_info* target_module)
{
	auto found = storage.find(target_module);
	if (found != storage.end( ))
		return found->second;

	//---

	auto& entry = storage[target_module];

	//---

	runtime_assert(entry.empty( ), "Entry already filled!");

	const auto& exports = target_module->exports( );

	const auto create_fn = exports.at("CreateInterface").addr;
	const auto reg       = create_fn.rel32(0x5).add(0x6).deref(2).ptr<CInterfaceRegister>( );

	auto temp_entry = std::vector<ifcs_entry_type::value_type>( );
	for (auto r = reg; r != nullptr; r = r->next)
		temp_entry.emplace_back(make_pair(std::string_view(r->name), r->create_fn));

	const auto contains_duplicate = [&](const std::string_view& new_string, size_t original_size)
	{
		auto detected = false;
		for (auto& e : temp_entry /*| std::views::keys*/)
		{
			auto&& raw_string = e.first;
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
			if (const auto str2 = std::string_view(str.begin( ), str.end( ) - 1); !contains_duplicate(str2, original_size))
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

	for (const auto& [name, fn] : temp_entry)
	{
		const auto name_pretty = get_pretty_string(name);
		entry.emplace(name_pretty.value_or(name), fn);
	}

	return entry;
}

nstd::address detail::find_csgo_interface(module_info* from, const std::string_view& target_name)
{
	static ifcs_storage_type storage;

	const auto& entry = _Interface_entry(storage, from);
	//const auto& fn = entry.at(interface_name);

	const auto found = entry.find(target_name);
	runtime_assert(found != entry.end( ));

	[[maybe_unused]] const auto debug_message = [&]
	{
		const auto original_interface_name     = found->first;
		const auto original_interface_name_end = original_interface_name._Unchecked_end( );

		auto msg = std::wostringstream( );
		msg << "Found interface: " << std::wstring(target_name.begin( ), target_name.end( )) << ' ';
		if (*original_interface_name_end != '\0')
		{
			msg
					<< '('
					<< std::wstring(original_interface_name.begin( ), original_interface_name.end( ))
					<< original_interface_name_end
					<< ") ";
		}

		msg
				<< "in module "
				<< from->name( );

		return msg;
	};
	CHEAT_CONSOLE_LOG(debug_message( ));

	return std::invoke(found->second);
}

void* detail::find_vtable_pointer(module_info* from, const std::string_view& class_name)
{
	auto& mgr = from->vtables( );

	const auto [addr] = mgr.at(class_name);
	[[maybe_unused]]
			const auto found_msg = [&]
			{
				const auto& from_name         = from->name( );
				const auto second_module_name = nstd::os::all_modules::get_ptr( )->rfind([&](const module_info& info)
				{
					return info.name( ) == from_name;
				});
				const auto module_name = second_module_name == from ? from_name : from->full_path( );
				return std::wostringstream( )
					   << "Found \"" << std::wstring(class_name.begin( ), class_name.end( ))
					   << "\" vtable in module \"" << module_name << '\"';
			};
	CHEAT_CONSOLE_LOG(found_msg( ));
	return addr.ptr<void>( );
}
