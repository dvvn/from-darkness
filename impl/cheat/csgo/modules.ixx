module;

#include <nstd/chars cache.h>
#include <nstd/format.h>

#include <windows.h>
#include <winternl.h>

#include <memory>
#include <string>
#include <functional>

export module cheat.csgo.modules;
import cheat.tools.object_name;
import nstd.winapi.exports;
import nstd.winapi.sections;
import nstd.winapi.vtables;
export import nstd.mem.address;

void console_log(const std::string_view module_name, const std::string_view object_type, const std::string_view object_name, const nstd::mem::basic_address<void> object_ptr);
void console_log(const std::function<std::string_view( )> module_name_getter, const std::string_view object_type, const std::string_view object_name, const nstd::mem::basic_address<void> object_ptr);

struct logs_writer
{
	void operator()(LDR_DATA_TABLE_ENTRY* entry, const std::string_view module_name) const;

	template<typename Fn>
	void operator()(const wp::found_export<Fn> ex, const std::string_view module_name, const std::string_view export_name) const
	{
		console_log(module_name, "export", export_name, ex.unknown);
	}

	void operator()(IMAGE_SECTION_HEADER* const sec, const std::string_view module_name, const std::string_view section_name) const;

	template<typename T>
	void operator()(const wp::found_vtable<T> vt, const std::string_view module_name, const std::string_view vtable_name) const
	{
		console_log(module_name, "vtable", vtable_name, vt.ptr);
	}
};

uint8_t* find_signature_impl(LDR_DATA_TABLE_ENTRY* ldr_entry, const std::string_view sig);
void* find_interface_impl(LDR_DATA_TABLE_ENTRY* ldr_entry, const nstd::mem::basic_address<void> create_interface_fn, const std::string_view name);

namespace wp = nstd::winapi;

template<nstd::chars_cache Name>
struct game_module
{
	template<typename Fn, nstd::chars_cache Export>
	Fn find_export( ) const
	{
		return wp::find_export<Fn, Name, Export, logs_writer>( );
	}

	template<nstd::chars_cache Section>
	auto find_section( ) const
	{
		return wp::find_section<Name, Section, logs_writer>( );
	}

#if 0
	template<typename T, nstd::chars_cache Class>
	T* find_vtable( ) const
	{
		return wp::find_vtable<T, Name, Class, logs_writer>( );
	}
#else
	template<typename T>
	T* find_vtable( ) const
	{
		static nstd::mem::basic_address<T> found = wp::find_vtable_impl<logs_writer>(wp::find_module<Name, logs_writer>( ),
																							   Name.view( ),
																							   cheat::tools::csgo_object_name<T>( ));
		return found.pointer;
	}
#endif

	template<nstd::chars_cache Sig>
	auto find_signature( )const
	{
		static nstd::mem::basic_address found = find_signature_impl(wp::find_module<Name, logs_writer>( ),
																	Sig.view( ));
		return found;
	}

	template<nstd::chars_cache IfcName>
	auto find_interface( ) const
	{
		static nstd::mem::basic_address found = find_interface_impl(wp::find_module<Name, logs_writer>( ),
																	find_export<void*, "CreateInterface">( ),
																	IfcName.view( ));
		return found;
	}

	//----

	template<typename T>
	void log_found_interface(T* ptr)const
	{
		console_log(Name.view( ), "interface", cheat::tools::csgo_object_name<T>( ), ptr);
	}
};

export namespace cheat::csgo_modules
{
#define CHEAT_GAME_MODULE(_NAME_)\
	inline constexpr auto _NAME_ = game_module<#_NAME_".dll">();

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
}
