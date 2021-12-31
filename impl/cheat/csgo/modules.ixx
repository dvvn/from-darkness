module;

#include <nstd/type name.h>
//unordered_map included here
#include <nstd/rtlib/includes.h>
#include <nstd/unordered_set.h>
#include <memory>

export module cheat.csgo_modules;
import cheat.csgo.structs.AppSystem;
import nstd.rtlib;

export namespace cheat
{
	template <typename T>
	// ReSharper disable once CppInconsistentNaming
	_INLINE_VAR constexpr auto CSGO_class_name = []
	{
		constexpr auto name = nstd::type_name<T>;
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
}

namespace cheat::csgo_modules
{
	using instance_fn = cheat::csgo::InstantiateInterfaceFn;
	using nstd::rtlib::info;

	class CInterfaceRegister
	{
	public:
		instance_fn create_fn;
		const char* name;
		CInterfaceRegister* next;
	};

	using ifcs_entry_type = nstd::unordered_map<std::string_view, instance_fn>;

	export struct game_module_storage
	{
		game_module_storage(const std::string_view& name);
		~game_module_storage( );

		game_module_storage(const game_module_storage& other) = delete;
		game_module_storage(game_module_storage&& other) noexcept;
		game_module_storage& operator=(const game_module_storage& other) = delete;
		game_module_storage& operator=(game_module_storage&& other) noexcept;

		nstd::address find_signature(const std::string_view& sig);
		void* find_vtable(const std::string_view& class_name);
		nstd::address find_game_interface(const std::string_view& ifc_name);

		template <typename Table>
		Table* find_vtable( )
		{
			constexpr auto table_name = CSGO_class_name<Table>.view( );
			void* ptr = find_vtable(table_name);
			return static_cast<Table*>(ptr);
		}

		void clear_interfaces_cache( );

	private:
		info* info_ptr;
#ifdef _DEBUG
		nstd::unordered_set<std::string> sigs_tested;
#endif
#ifdef CHEAT_HAVE_CONSOLE
		nstd::unordered_set<std::string> vtables_tested;
#endif
		ifcs_entry_type interfaces;
	};

	export void reset_interfaces_storage( );

	export struct game_module_base
	{
		constexpr game_module_base(const std::string_view& name, size_t index)
			: name_(name), index_(index)
		{
		}

		game_module_storage* operator->( ) const;

	private:
		std::string_view name_;
		size_t index_;
	};

#define CHEAT_GAME_MODULE(_NAME_)\
	export _INLINE_VAR constexpr auto _NAME_ = game_module_base(#_NAME_,__LINE__ - _first_line )

	// ReSharper disable once CppInconsistentNaming
	constexpr auto _first_line = __LINE__ + 1;
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

#undef CHEAT_GAME_MODULE
}
