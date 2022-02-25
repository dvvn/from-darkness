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

template<typename T>
static decltype(auto) _Transform_wide(T&& rng)
{
	if constexpr (std::same_as<std::ranges::range_value_t<T>, wchar_t>)
		return std::forward<T>(rng);
	else
		return std::views::transform(std::forward<T>(rng), nstd::text::cast_all<wchar_t>);
}

template<typename ...Args>
static constexpr auto _Wide(Args&&...args)
{
	return nstd::container::append<std::wstring>(_Transform_wide(std::span(args...)));
}

static console* _Get_console( )
{
	return services_loader::get( ).deps( ).try_get<console>( );
}

static rtlib::info* _Find_modue(const rtlib::info_string::fixed_type& str)
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
	for (auto chr : target_name)
	{
		if (chr == static_cast<decltype(chr)>('.'))
			return _Find_modue(target_name);
	}

	using namespace std::string_view_literals;
	const auto wstr = nstd::container::append<std::wstring>(_Transform_wide(target_name), L".dll"sv);
	return _Find_modue(wstr);
}

//not thread-safe

using instance_fn = void* (*)();
struct interfaces_storage : std::vector<std::pair<std::string_view, instance_fn>>
{
	template<typename Fn>
	auto call(const std::string_view name, const Fn callback)const
	{
		const auto compare = [=](const value_type other)
		{
#ifdef __cpp_lib_string_contains
			return other.first.contains(name);
#else
			return other.first.find(name) != other.first.npos;
#endif
		};

		const auto bg = this->begin( );
		const auto ed = this->end( );

		const auto found = std::find_if(bg, ed, std::ref(compare));
		runtime_assert(found != ed, "Unable to find interface with given name!");
		runtime_assert(std::none_of(std::next(found), ed, std::ref(compare)), "Found multiple interfaces with given name!");

		std::invoke(callback, *found);
		return std::invoke(found->second);
	}
};
struct interfaces_storage_pretty : nstd::unordered_map<std::string_view, instance_fn>
{
	template<typename Fn>
	auto call(const std::string_view name, const Fn callback)const
	{
		const auto found = this->find(name);
		runtime_assert(found != this->end( ), "Unable to find interface with given name!");

		std::invoke(callback, *found);
		return std::invoke(found->second);
	}
};
struct interface_reg
{
	instance_fn create_fn;
	const char* name;
	interface_reg* next;
};

static auto _Find_interfaces(rtlib::info* target_module)
{
	constexpr std::string_view export_name = "CreateInterface";

	const auto log_fn = [=](rtlib::export_data* e, bool created)->void
	{
		const auto _Console = _Get_console( );
		if (!_Console)
			return;

		const std::wstring_view dllname = target_module->name.raw;
		const auto wexport_name = _Wide(export_name);
		if (created)
			_Console->log(L"{} -> export \"{}\" at {:#X} found", dllname, wexport_name, e->addr.value);
		else if (!e)
			_Console->log(L"{} -> export \"{}\" not found", dllname, wexport_name);
	};

	const auto create_fn = target_module->exports( ).at(export_name, std::ref(log_fn)).addr;

	const interface_reg* reg = create_fn./*rel32*/jmp(0x5).plus(0x6).deref<2>( );

	interfaces_storage storage;
	for (auto r = reg; r != nullptr; r = r->next)
		storage.emplace_back(r->name, r->create_fn);

	return storage;
}

static auto _Find_interfaces_pretty(rtlib::info* target_module)
{
	const auto temp_entry = _Find_interfaces(target_module);

	const auto contains_duplicate = [&](const std::string_view new_string, size_t max_size)
	{
		bool detected = false;
		for (const auto raw_string : temp_entry | std::views::keys)
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
	const auto get_pretty_string = [&](const std::string_view str) -> std::optional<std::string_view>
	{
#if 1
		const auto remove = std::ranges::distance(str | std::views::reverse | std::views::take_while([](unsigned char c) { return std::isdigit(c); }));
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

	interfaces_storage_pretty storage;
	storage.reserve(temp_entry.size( ));

	for (const auto [name, fn] : temp_entry)
	{
		const auto name_pretty = get_pretty_string(name);
		storage.emplace(name_pretty.value_or(name), fn);
	}

	return storage;
}

struct interfaces_storage_any
{
	using value_type = std::variant<interfaces_storage, interfaces_storage_pretty>;
	value_type storage_;

public:

	interfaces_storage_any( ) = default;

	interfaces_storage_any(rtlib::info* info, bool pretty)
	{
		if (pretty)
			storage_ = _Find_interfaces_pretty(info);
		else
			storage_ = _Find_interfaces(info);
	}

	template<typename Fn>
	auto call(const std::string_view name, const Fn callback)const
	{
		return std::visit([=]<class Storage>(const Storage & s)
		{
			return s.call(name, callback);
		}, storage_);
	}

	void swap(interfaces_storage_any& other)noexcept
	{
		constexpr auto do_move = [](value_type& current, value_type& other)
		{
			using std::swap;
			using nstd::swap;
			std::visit([&]<class Storage>(Storage & unwrapped)
			{
				current = std::move(unwrapped);
			}, other);
		};

		value_type temp;
		do_move(temp, other.storage_);
		do_move(other.storage_, storage_);
		do_move(storage_, temp);
		/*temp = std::move(other.storage_);
		other = std::move(storage_);
		storage_ = std::move(temp);*/
	}
};

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
		bool operator()(const std::string_view sig)
		{
			const auto lock = std::scoped_lock(mtx_);
			const auto created = cache_.emplace(sig).second;
			return created == false;
		}
		bool operator()(const std::string_view sig)const
		{
			return cache_.contains(sig);
		}
	};

	rtlib::info* info_ptr = nullptr;
	_String_tester<std::mutex> sigs_tested;
	//_String_tester<_Fake_mutex> vtables_tested;
	interfaces_storage_any interfaces;

	module_storage_data(const std::string_view name)
	{
		info_ptr = _Get_module(name);
		interfaces = {info_ptr,true};
	}
};

data_extractor::data_extractor(storage_type storage)
	:storage_(storage)
{
}

address data_extractor::find_signature(const std::string_view sig)
{
	std::wstring_view log_name;
	std::wstring log_sig;
	const auto _Console = _Get_console( );
	if (_Console)
	{
		log_name = storage_->info_ptr->name.raw;
		log_sig = _Wide(sig);

		if (storage_->sigs_tested(sig))
		{
			_Console->log(L"{} -> signature \"{}\": found before!", log_name, log_sig);
			return nullptr;
		}
		_Console->log(L"{} -> signature \"{}\": searching...", log_name, log_sig);
	};

	//no cache inside

	using namespace nstd::mem;
	const auto block = storage_->info_ptr->mem_block( );
	const auto bytes = make_signature(sig.begin( ), sig.end( ), signature_convert( ));
	const auto ret = block.find_block(bytes);

	if (_Console)
	{
		const auto postfix = ret.empty( ) ? L" NOT " : L" ";
		_Console->log(L"{} -> signature \"{}\":{}found", log_name, log_sig, postfix);
	};

	return ret.data( );
}

void* data_extractor::find_vtable(const std::string_view class_name)
{
	auto& mgr = storage_->info_ptr->vtables( );
	const auto vt = mgr.at(class_name, [=]<class C>(C*, bool created)
	{
		if (!created)
			return;

		const auto _Console = _Get_console( );
		if (!_Console)
			return;

		using namespace nstd::rtlib;
		const auto infos = all_infos::get( ) | std::views::reverse;
		const info_string& from_name = storage_->info_ptr->name;

		const auto module_with_same_name = std::ranges::find(infos, from_name, &info::name);

		std::wstring_view module_name;
		if (*module_with_same_name == *storage_->info_ptr)
			module_name = from_name.raw;
		else
			module_name = storage_->info_ptr->full_path.raw;

		_Console->log(L"{} -> vtable \"{}\" found", module_name, _Wide(class_name));
	});

	return vt.addr;
}

static std::wstring _Get_full_interface_name(const char* begin, size_t known_size)
{
	std::wstring ret;
	if (const auto known_end = begin + known_size; *known_end != '\0')
	{
		const auto hidden_size = std::char_traits<char>::length(known_end);
		ret = std::format(L" (full name \"{}\")", _Wide(begin, known_size + hidden_size));
	}
	return ret;
}

address data_extractor::find_game_interface(const std::string_view ifc_name)
{
	const auto callback = [=]<class Pair>(const Pair found)
	{
		static_assert(std::is_trivially_copyable_v<Pair>, "Change pair type to reference!");

		const auto _Console = _Get_console( );
		if (!_Console)
			return;

		_Console->log(L"{} -> interface \"{}\"{}: found"
					  , storage_->info_ptr->name.raw
					  , _Wide(ifc_name)
					  , _Get_full_interface_name(found.first.data( ), ifc_name.size( )));

	};

	return storage_->interfaces.call(ifc_name, std::ref(callback));
}

void data_extractor::clear_interfaces_cache( )
{
	//'clear' not called because we also want to free memory
	interfaces_storage_any tmp;
	tmp.swap(storage_->interfaces);
}

struct module_storage
{
	module_storage_data storage;
	data_extractor extractor;

	module_storage(const std::string_view name)
		:storage(name), extractor(std::addressof(storage))
	{
	}
};

class storage_getter
{
	std::optional<module_storage> storage_;
	std::mutex mtx_;

public:
	data_extractor* get(const std::string_view name)
	{
		if (!storage_.has_value( ))
		{
			const auto lock = std::scoped_lock(mtx_);
			if (!storage_.has_value( ))
				storage_.emplace(name);
		}
		return std::addressof(storage_->extractor);
	}

	const data_extractor* get( )const
	{
		return std::addressof(storage_->extractor);
	}
};

struct modules_database : std::array<storage_getter, modules_count>
{
};

data_extractor* cheat::csgo_modules::get(const std::string_view name, size_t index)
{
	return nstd::one_instance<modules_database>::get( )[index].get(name);
}

data_extractor* game_module_base::get( ) const
{
	return csgo_modules::get(name, index);
}

data_extractor* game_module_base::operator->( ) const
{
	return this->get( );
}

