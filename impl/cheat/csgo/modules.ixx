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

using nstd::mem::basic_address;

using module_name_getter_fn = std::function<std::string_view( )>;

void console_log(const std::string_view module_name, const std::string_view object_type, const std::string_view object_name, const basic_address<void> object_ptr) noexcept;
void console_log(const module_name_getter_fn module_name_getter, const std::string_view object_type, const std::string_view object_name, const basic_address<void> object_ptr) noexcept;

namespace wp = nstd::winapi;

struct logs_writer
{
	void operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view module_name) const noexcept;

	template<typename FnT>
	void operator()(const wp::found_export<FnT> ex, const std::string_view module_name, const std::string_view export_name) const noexcept
	{
		console_log(module_name, "export", export_name, ex.unknown);
	}

	void operator()(IMAGE_SECTION_HEADER* const sec, const std::string_view module_name, const std::string_view section_name) const noexcept;

	template<typename T>
	void operator()(const wp::found_vtable<T> vt, const std::string_view module_name, const std::string_view vtable_name) const noexcept
	{
		console_log(module_name, "vtable", vtable_name, vt.ptr);
	}
};

uint8_t* find_signature_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig) noexcept;
void* find_interface_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const basic_address<void> create_interface_fn, const std::string_view name) noexcept;

using cheat::tools::csgo_object_name;

template<nstd::chars_cache Name>
struct game_module
{
	template<typename FnT, nstd::chars_cache Export>
	FnT find_export( ) const noexcept
	{
		return wp::find_export<FnT, Name, Export, logs_writer>( );
	}

	template<nstd::chars_cache Section>
	auto find_section( ) const noexcept
	{
		return wp::find_section<Name, Section, logs_writer>( );
	}

	template<typename T>
	T* find_vtable( ) const noexcept
	{
		static const basic_address<T> found = wp::find_vtable<logs_writer>(wp::find_module<Name, logs_writer>( ),
																		   Name.view( ),
																		   csgo_object_name<T>( ));
		return found.pointer;
	}

	template<nstd::chars_cache Sig>
	auto find_signature( ) const noexcept
	{
		static const basic_address found = find_signature_impl(wp::find_module<Name, logs_writer>( ),
															   Sig.view( ));
		return found;
	}

	template<nstd::chars_cache IfcName>
	auto find_interface( ) const noexcept
	{
		static basic_address found = find_interface_impl(wp::find_module<Name, logs_writer>( ),
														 this->find_export<void*, "CreateInterface">( ),
														 IfcName.view( ));
		return found;
	}

	//----

	template<typename T>
	void log_found_interface(T* ptr) const noexcept
	{
		console_log(Name.view( ), "interface", csgo_object_name<T>( ), ptr);
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
