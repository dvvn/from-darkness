module;

#include "modules_includes.h"

export module cheat.csgo.modules;
export import nstd.rtlib;

template <typename T>
inline constexpr auto CSGO_class_name_holder = []
{
	constexpr auto name = nstd::type_name<T>( );
	constexpr std::string_view drop = "cheat::csgo";
#if 1
	constexpr auto buffer = nstd::drop_namespace(name, drop);
	if constexpr (buffer.ideal( ))
		return buffer;
	else
		return buffer.make_ideal<buffer.str_size>( );
#else

	constexpr auto out_size = nstd::drop_namespace(name, drop).size( );
	const auto out = nstd::drop_namespace(name, drop);
	return nstd::string_to_buffer<out_size>(out);
#endif
}();

export namespace cheat
{
	template <typename T>
	constexpr auto CSGO_class_name( )
	{
		return CSGO_class_name_holder<T>.view( );
	}
}

namespace cheat::csgo_modules
{
	using nstd::rtlib::info;
	using nstd::mem::address;

	typedef void* (*instance_fn)();
	//not thread-safe
	using ifcs_entry_type = nstd::unordered_map<std::string_view, instance_fn>;

	//todo: rename
	export struct game_module_storage
	{
		using storage_type = void*;

		game_module_storage(storage_type data);
		game_module_storage(const game_module_storage& other) = delete;
		game_module_storage& operator=(const game_module_storage& other) = delete;

		address find_signature(std::string_view sig);
		void* find_vtable(std::string_view class_name);
		address find_game_interface(std::string_view ifc_name);

		template <typename Table>
		Table* find_vtable( )
		{
			constexpr auto table_name = CSGO_class_name<Table>( );
			void* ptr = find_vtable(table_name);
			return static_cast<Table*>(ptr);
		}

		void clear_interfaces_cache( );

	private:
		storage_type data_;
	};

	export game_module_storage* get(std::string_view name, size_t index);

	export struct game_module_base
	{
		constexpr game_module_base(std::string_view name, size_t index)
			: name(name), index(index)
		{
		}

		game_module_storage* get( ) const;
		game_module_storage* operator->( ) const;

		std::string_view name;
		size_t index;
	};

#define CHEAT_GAME_MODULE(_NAME_)\
	export inline constexpr game_module_base _NAME_ = {#_NAME_, __LINE__ - _first_line}

	inline constexpr auto _first_line = __LINE__ + 1;
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
	inline constexpr auto modules_count = __LINE__ - _first_line;
}
