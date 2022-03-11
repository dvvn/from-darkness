module;

#include <nstd/rtlib/includes.h>
#include <nstd/mem/block_includes.h>
#include <nstd/unordered_set.h>
#include <nstd/unordered_map.h>
#include <nstd/format.h>

#include <variant>
#include <filesystem>
#include <condition_variable>

module cheat.csgo.modules;
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

static constexpr auto _Wide = []<typename ...Args>(Args&&...args)
{
	return nstd::container::append<std::wstring>(_Transform_wide(std::span(args...)));
};

class modules_accesser
{
	using mutex_type = std::recursive_mutex;
	using lock_type = std::scoped_lock<mutex_type>;
	lock_type lock_;

	using mtx = nstd::one_instance<mutex_type, __LINE__>;

public:
	modules_accesser(/*bool lock = true*/)
		:lock_(mtx::get( ))
	{
	}

	//lock and unlock for conditional variable only

	void lock( )noexcept
	{
		mtx::get( ).lock( );
	}

	void unlock( )noexcept
	{
		mtx::get( ).unlock( );
	}

	auto begin( )const
	{
		return rtlib::all_infos::get( ).begin( );
	}

	auto end( )const
	{
		return rtlib::all_infos::get( ).end( );
	}

	auto operator->( )const
	{
		return rtlib::all_infos::get_ptr( );
	}
};

static void _Init_modules( )
{
	const modules_accesser modules;
	runtime_assert(!modules->contains_locker( ) && modules->empty( ), "Modules storage already updated!");
	modules->set_locker([](auto& unused) {return false; });
	modules->update(false);
	const auto& owner_path = modules->owner( ).work_dir.fixed;
	const nstd::hashed_wstring str = const_cast<std::wstring&&>(
		std::filesystem::path(owner_path.begin( ), owner_path.end( )).append(L"bin").append(L"serverbrowser.dll").native( ));

	modules->set_locker([hash = str.hash( )](const rtlib::modules_storage_data& data)
	{
		for (const auto& m : data | std::views::reverse)
		{
			if (m.full_path.fixed.hash( ) == hash)
				return true;
		}
		return false;
	});
}

//thread-safe modules updater
static rtlib::info* _Find_modue(const rtlib::info_string::fixed_type& str)
{
	[[maybe_unused]]
	static const uint8_t init = (_Init_modules( ), 0);

	for (;;)
	{
		for (auto& entry : modules_accesser( ))
		{
			if (entry.name == str)
				return std::addressof(entry);
		}

		modules_accesser accesser;
		if (accesser->locked( ))
			return nullptr;

		std::condition_variable_any cv;
		cv.wait(accesser, []
		{
			return rtlib::all_infos::get( ).update(true);
		});
	}
}

template <std::ranges::random_access_range Rng>
static auto _Find_extension(const Rng& rng)
{
	using val_t = std::ranges::range_value_t<Rng>;
	std::basic_string_view<val_t> out = {rng.begin( ),rng.end( )};

	const auto offset = out.rfind(static_cast<val_t>('.'));
	if (offset != out.npos)
		out.remove_prefix(offset);
	return out;
}

template <class Rng>
static auto _Get_module(const Rng& target_name)
{
	rtlib::info* ret;

	const auto extension = _Find_extension(target_name);
	if (!extension.empty( ))
	{
		ret = _Find_modue(target_name);
	}
	else
	{
		const auto wstr = nstd::container::append<std::wstring>(_Transform_wide(target_name), _Transform_wide(extension));
		ret = _Find_modue(wstr);
	}

	return ret;
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
struct interfaces_storage_hashed : nstd::unordered_map<std::string_view, instance_fn>
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
#if 0
	constexpr std::string_view export_name = "CreateInterface";

	const auto log_fn = [=](rtlib::export_data* e, bool created)->void
	{
		if (!console::active( ))
			return;

		const std::wstring_view dllname = target_module->name.raw;
		const auto wexport_name = _Wide(export_name);
		if (created)
			console::log(L"{} -> export \"{}\" at {:#X} found", dllname, wexport_name, e->addr.value);
		else if (!e)
			console::log(L"{} -> export \"{}\" not found", dllname, wexport_name);
	};

	const auto create_fn = target_module->exports( ).at(export_name, std::ref(log_fn)).addr;

	const interface_reg* reg = create_fn./*rel32*/jmp(0x5).plus(0x6).deref<2>( );

	interfaces_storage storage;
	for (auto r = reg; r != nullptr; r = r->next)
		storage.emplace_back(r->name, r->create_fn);

	return storage;
#endif
	runtime_assert("nstd::rtlib deprecated");
	return interfaces_storage( );
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

	interfaces_storage storage;
	storage.reserve(temp_entry.size( ));

	for (const auto [name, fn] : temp_entry)
	{
		const auto name_pretty = get_pretty_string(name);
		storage.emplace_back(name_pretty.value_or(name), fn);
	}

	return storage;
}

struct interfaces_storage_any
{
	using value_type = std::variant<interfaces_storage, interfaces_storage_hashed>;
	value_type storage_;

public:

	interfaces_storage_any( ) = default;

	interfaces_storage_any(value_type&& storage)
		:storage_(std::move(storage))
	{
	}

	interfaces_storage_any& operator=(value_type&& storage)
	{
		swap(storage);
		return *this;
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
		swap(other.storage_);
	}

private:
	void swap(value_type& other)noexcept
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
		do_move(temp, other);
		do_move(other, storage_);
		do_move(storage_, temp);
	}
};

static interfaces_storage_any _Store_interfaces(rtlib::info* target_module, bool store_full_interface_names, bool store_interfaces_in_hashtable)
{
	interfaces_storage storage = std::invoke(store_full_interface_names ? _Find_interfaces : _Find_interfaces_pretty, target_module);
	interfaces_storage_any out;
	if (store_interfaces_in_hashtable)
		out = nstd::container::append<interfaces_storage_hashed>(storage);
	else
		out = std::move(storage);
	return out;
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

	module_storage_data(const std::string_view name, bool store_full_interface_names, bool store_interfaces_in_hashtable)
	{
		info_ptr = _Get_module(name);
		interfaces = _Store_interfaces(info_ptr, store_full_interface_names, store_interfaces_in_hashtable);
	}
};

data_extractor::data_extractor(storage_type storage)
	:storage_(storage)
{
}

address data_extractor::find_signature(const std::string_view sig)
{
#if 0
	std::wstring_view log_name;
	std::wstring log_sig;
	if (console::active( ))
	{
		log_name = storage_->info_ptr->name.raw;
		log_sig = _Wide(sig);

		if (storage_->sigs_tested(sig))
		{
			console::log(L"{} -> signature \"{}\": found before!", log_name, log_sig);
			return nullptr;
		}
		console::log(L"{} -> signature \"{}\": searching...", log_name, log_sig);
	};

	//no cache inside

	using namespace nstd::mem;
	const auto block = storage_->info_ptr->mem_block( );
	const auto bytes = make_signature(sig.begin( ), sig.end( ), signature_convert( ));
	const auto ret = block.find_block(bytes);

	if (console::active( ))
	{
		const auto postfix = ret.empty( ) ? L" NOT " : L" ";
		console::log(L"{} -> signature \"{}\":{}found", log_name, log_sig, postfix);
	};

	return ret.data( );
#endif

	runtime_assert("nstd::rtlib deprecated");
	return nullptr;
}

void* data_extractor::find_vtable(const std::string_view class_name)
{
#if 0
	auto& mgr = storage_->info_ptr->vtables( );
	const auto vt = mgr.at(class_name, [=]<class C>(C*, bool created)
	{
		if (!created)
			return;
		if (!console::active( ))
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

		console::log(L"{} -> vtable \"{}\" found", module_name, _Wide(class_name));
	});

	return vt.addr;
#endif

	runtime_assert("nstd::rtlib deprecated");
	return 0;
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
#if 0
	const auto callback = [=]<class Pair>(const Pair found)
	{
		static_assert(std::is_trivially_copyable_v<Pair>, "Change pair type to reference!");

		if (!console::active( ))
			return;

		console::log(L"{} -> interface \"{}\"{}: found"
					 , storage_->info_ptr->name.raw
					 , _Wide(ifc_name)
					 , _Get_full_interface_name(found.first.data( ), ifc_name.size( )));

	};

	return storage_->interfaces.call(ifc_name, std::ref(callback));
#endif

	runtime_assert("nstd::rtlib deprecated");
	return nullptr;
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

	template<class ...Args> //yes im lazy
	module_storage(const Args...args)
		:storage(args...), extractor(std::addressof(storage))
	{
	}
};

class storage_getter
{
	std::optional<module_storage> storage_;
	std::mutex mtx_;

public:
	template<class ...Args>
	data_extractor* get(const Args...args)
	{
		if (!storage_.has_value( ))
		{
			const auto lock = std::scoped_lock(mtx_);
			if (!storage_.has_value( ))
				storage_.emplace(args...);
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

data_extractor* game_module_base::get( ) const
{
	return nstd::one_instance<modules_database>::get( )[index].get(name, store_full_interface_names, store_interfaces_in_hashtable);
}

data_extractor* game_module_base::operator->( ) const
{
	return this->get( );
}

