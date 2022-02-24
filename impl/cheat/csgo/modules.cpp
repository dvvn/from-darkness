module;

#include "cheat/console/includes.h"

#include <nstd/rtlib/includes.h>
#include <nstd/mem/block_includes.h>
#include <nstd/unordered_set.h>
#include <nstd/unordered_map.h>

module cheat.csgo.modules;
import cheat.csgo.interfaces;
import cheat.root_service;
import cheat.console;
import nstd.rtlib;
import nstd.container.wrapper;
import nstd.text.actions;

using namespace cheat;
using namespace csgo_modules;
using nstd::mem::address;
namespace rtlib = nstd::rtlib;

template<typename ...Args>
static constexpr auto _Wide(Args&&...args)
{
	using namespace nstd;
	auto tmp = std::views::transform(std::span(args...), text::cast_all<wchar_t>);
	return container::append<std::wstring>(tmp);
}

static console* _Get_console( )
{
	return services_loader::get( ).deps( ).try_get<console>( );
}

using fixed_rtstring = rtlib::info_string::fixed_type;
static rtlib::info* _Find_modue(const fixed_rtstring& str)
{
	for (rtlib::info& entry : rtlib::all_infos::get( ))
	{
		if (entry.name == str)
			return std::addressof(entry);
	}
	return nullptr;
}

template <class Rng>
static rtlib::info* _Get_module(const Rng& target_name)
{
	if (target_name.rfind(static_cast<Rng::value_type>('.')) == target_name.npos)
	{
		using namespace nstd;
		using namespace std::string_view_literals;
		const auto wstr = container::append<std::wstring>(target_name | std::views::transform(text::cast_all<wchar_t>), L".dll"sv);
		return _Find_modue(wstr);
	}

	return _Find_modue(target_name);
}

static ifcs_entry_type _Interface_entry(rtlib::info* target_module)
{
	struct interface_register
	{
		instance_fn create_fn;
		const char* name;
		interface_register* next;
	};

	constexpr std::string_view export_name = "CreateInterface";

	const auto log_fn = [&](rtlib::export_data* e, bool created)->void
	{
		using namespace cheat;
		const auto _Console = _Get_console( );
		if (_Console)
		{
			const std::wstring_view dllname = target_module->name.raw;
			if (created)
				_Console->log(L"{} -> export \"{}\" at {:#X} found", dllname, _Wide(export_name), e->addr.value);
			else if (!e)
				_Console->log(L"{} -> export \"{}\" not found", dllname, _Wide(export_name));
		}
	};

	const auto create_fn = target_module->exports( ).at(export_name, log_fn).addr;

	const interface_register* reg = create_fn./*rel32*/jmp(0x5).plus(0x6).deref<2>( );

	std::vector<ifcs_entry_type::value_type> temp_entry;
	for (auto r = reg; r != nullptr; r = r->next)
		temp_entry.emplace_back(r->name, r->create_fn);

	const auto contains_duplicate = [&](std::string_view new_string, size_t max_size)
	{
		bool detected = false;
		for (auto& raw_string : temp_entry | std::views::keys)
		{
			if (raw_string.size( ) >= max_size)
				continue;

			if (!raw_string.starts_with(new_string))
				continue;
			if (raw_string.size( ) == new_string.size( ))
				continue;

			if (!detected)
				detected = true;
			else
				return true;
		}
		return false;
	};
	const auto drop_underline = [&](std::string_view str, size_t max_size) -> std::optional<std::string_view>
	{
		if (str.ends_with('_'))
		{
			/*const auto str2 = str.substr(0, str.size( ) - 1);
			if (!contains_duplicate(str2, max_size))
				return str2;*/
			str.remove_suffix(1);
			if (!contains_duplicate(str, max_size))
				return str;

		}
		return {};
	};
	const auto get_pretty_string = [&](std::string_view str) -> std::optional<std::string_view>
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
		const auto str_size = str.size( );

		if (remove != 0)
		{
			const auto str2 = str.substr(0, str.size( ) - remove);
			if (!contains_duplicate(str2, str_size))
				return drop_underline(str2, str_size).value_or(str2);
		}
		return drop_underline(str, str_size);
	};

	ifcs_entry_type entry;

	for (const auto [name, fn] : temp_entry)
	{
		const auto name_pretty = get_pretty_string(name);
		entry.emplace(name_pretty.value_or(name), fn);
	}

	return entry;
}

struct module_storage_data
{
	/*struct _Fake_mutex
	{
		void lock( ) noexcept { }
		void unlock( ) noexcept { }
	};*/

	template<class Mtx>
	class _String_tester
	{
		Mtx mtx_;
		nstd::unordered_set<std::string> cache_;
	public:
		bool operator()(std::string_view sig)
		{
			const auto lock = std::scoped_lock(mtx_);
			const auto created = cache_.emplace(sig).second;
			return created == false;
		}
		bool operator()(std::string_view sig)const
		{
			return cache_.contains(sig);
		}
	};

	rtlib::info* info_ptr = nullptr;
	_String_tester<std::mutex> sigs_tested;
	//_String_tester<_Fake_mutex> vtables_tested;
	ifcs_entry_type interfaces;

	module_storage_data(std::string_view name)
	{
		info_ptr = _Get_module(name);
		interfaces = _Interface_entry(info_ptr);
	}
};

game_module_storage::game_module_storage(storage_type data) :data_(data)
{
}

address game_module_storage::find_signature(std::string_view sig)
{
	const auto storage = static_cast<module_storage_data*>(data_);

	std::wstring_view log_name;
	std::wstring log_sig;
	const auto _Console = _Get_console( );
	if (_Console)
	{
		log_name = storage->info_ptr->name.raw;
		log_sig = _Wide(sig);

		if (storage->sigs_tested(sig))
		{
			_Console->log(L"{} -> signature \"{}\": found before!", log_name, log_sig);
			return nullptr;
		}
		_Console->log(L"{} -> signature \"{}\": searching...", log_name, log_sig);
	};

	//no cache inside

	using namespace nstd::mem;
	const auto block = storage->info_ptr->mem_block( );
	const auto bytes = make_signature(sig.begin( ), sig.end( ), signature_convert( ));
	const auto ret = block.find_block(bytes);

	if (_Console)
	{
		const auto postfix = ret.empty( ) ? L" NOT " : L" ";
		_Console->log(L"{} -> signature \"{}\":{}found", log_name, log_sig, postfix);
	};

	return ret.data( );
}

void* game_module_storage::find_vtable(std::string_view class_name)
{
	const auto storage = static_cast<module_storage_data*>(data_);

	auto& mgr = storage->info_ptr->vtables( );
	const auto vt = mgr.at(class_name, [&]<class C>(C*, bool created)
	{
		if (!created)
			return;

		const auto _Console = _Get_console( );
		if (!_Console)
			return;

		using namespace nstd::rtlib;
		const auto infos = all_infos::get( ) | std::views::reverse;
		const info_string& from_name = storage->info_ptr->name;

		const auto module_with_same_name = std::ranges::find(infos, from_name, &info::name);

		std::wstring_view module_name;
		if (*module_with_same_name == *storage->info_ptr)
			module_name = from_name.raw;
		else
			module_name = storage->info_ptr->full_path.raw;

		_Console->log(L"{} -> vtable \"{}\" found", module_name, _Wide(class_name));
	});

	return vt.addr;
}

static std::wstring _Get_full_interface_name(const char* begin, size_t known_size)
{
	const auto known_end = begin + known_size;
	if (*known_end == '\0')
		return {};

	const auto hidden_size = std::char_traits<char>::length(known_end);
	return std::format(L" (full name \"{}\")", _Wide(begin, known_end + hidden_size));
}

address game_module_storage::find_game_interface(std::string_view ifc_name)
{
	const auto storage = static_cast<module_storage_data*>(data_);

	const auto found = storage->interfaces.find(ifc_name);
	runtime_assert(found != storage->interfaces.end( ));

	const auto _Console = _Get_console( );
	if (_Console)
	{
		_Console->log(L"{} -> interface \"{}\"{}: found"
					  , storage->info_ptr->name.raw
					  , _Wide(ifc_name)
					  , _Get_full_interface_name(found->first.data( ), ifc_name.size( ))
		);
	}

	return std::invoke(found->second);
}

void game_module_storage::clear_interfaces_cache( )
{
	const auto storage = static_cast<module_storage_data*>(data_);

	//'clear' not called because we also want to free memory
	ifcs_entry_type tmp;
	tmp.swap(storage->interfaces);
}

struct module_storage
{
	module_storage_data storage;
	game_module_storage unnamed;

	module_storage(std::string_view name)
		:storage(name), unnamed(std::addressof(storage))
	{
	}
};

class storage_getter
{
	std::optional<module_storage> data_;
	std::mutex builder_;

public:
	game_module_storage* get(std::string_view name)
	{
		if (!data_.has_value( ))
		{
			const auto lock = std::scoped_lock(builder_);
			data_.emplace(name);
		}
		return std::addressof(data_->unnamed);
	}

	const game_module_storage* get( )const
	{
		return std::addressof(data_->unnamed);
	}
};

struct modules_database : std::array<storage_getter, modules_count>
{
};

game_module_storage* cheat::csgo_modules::get(std::string_view name, size_t index)
{
	return nstd::one_instance<modules_database>::get( )[index].get(name);
}

game_module_storage* game_module_base::get( ) const
{
	return csgo_modules::get(name, index);
}

game_module_storage* game_module_base::operator->( ) const
{
	return this->get( );
}

