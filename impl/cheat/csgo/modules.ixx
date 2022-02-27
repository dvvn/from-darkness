module;

#include <nstd/type name.h>
#include <memory>

export module cheat.csgo.modules;
export import nstd.mem.address;

template <typename T>
inline constexpr auto CSGO_class_name_holder = []
{
	constexpr auto name = nstd::type_name<T>( );
	constexpr std::string_view drop = "cheat::csgo";

	constexpr auto buffer = nstd::drop_namespace(name, drop);
	if constexpr (buffer.ideal( ))
		return buffer;
	else
		return buffer.make_ideal<buffer.str_size>( );
}();

struct module_storage_data;

namespace cheat::csgo_modules
{
	export template <typename T>
		constexpr auto CSGO_class_name( )
	{
		return CSGO_class_name_holder<T>.view( );
	}

	export struct data_extractor
	{
		using storage_type = module_storage_data*;

		data_extractor(storage_type storage);
		data_extractor(const data_extractor& other) = delete;
		data_extractor& operator=(const data_extractor& other) = delete;

		nstd::mem::address find_signature(const std::string_view sig);
		void* find_vtable(const std::string_view class_name);
		nstd::mem::address find_game_interface(const std::string_view ifc_name);

		template <typename Table>
		Table* find_vtable( )
		{
			constexpr auto table_name = CSGO_class_name<Table>( );
			void* ptr = find_vtable(table_name);
			return static_cast<Table*>(ptr);
		}

		void clear_interfaces_cache( );

	private:
		storage_type storage_;
	};

	export struct game_module_base
	{
		constexpr game_module_base(const std::string_view name, size_t index, bool store_full_interface_names = false, bool store_interfaces_in_hashtable = true)
			: name(name), index(index), store_full_interface_names(store_full_interface_names), store_interfaces_in_hashtable(store_interfaces_in_hashtable)
		{
		}

		data_extractor* get( ) const;
		data_extractor* operator->( ) const;

		std::string_view name;
		size_t index;
		bool store_full_interface_names;
		bool store_interfaces_in_hashtable;
	};

#define CHEAT_GAME_MODULE(_NAME_,...)\
	export inline constexpr game_module_base _NAME_ = {#_NAME_, __LINE__ - modules_count_hint,__VA_ARGS__}

	inline constexpr auto modules_count_hint = __LINE__ + 1;
	CHEAT_GAME_MODULE(server);
	CHEAT_GAME_MODULE(client);
	CHEAT_GAME_MODULE(engine);
	CHEAT_GAME_MODULE(datacache);
	CHEAT_GAME_MODULE(materialsystem);
	CHEAT_GAME_MODULE(vstdlib);
	CHEAT_GAME_MODULE(vgui2);
	CHEAT_GAME_MODULE(vguimatsurface);
	CHEAT_GAME_MODULE(vphysics);
	CHEAT_GAME_MODULE(inputsystem);
	CHEAT_GAME_MODULE(studiorender);
	CHEAT_GAME_MODULE(shaderapidx9);
	inline constexpr auto modules_count = __LINE__ - modules_count_hint;
}
