module;

#ifdef _UNICODE
#include <nstd/core.h>
#endif
#include <nstd/format.h>

#include <windows.h>
#include <winternl.h>

#include <memory>
#include <string>

export module cheat.csgo.modules;
import cheat.tools.object_name;
import nstd.winapi.exports;
import nstd.winapi.sections;
import nstd.winapi.vtables;
export import nstd.mem.address;
import nstd.text.chars_cache;

using nstd::mem::basic_address;
namespace wp = nstd::winapi;
using wp::_Strv;

void console_log(const _Strv module_name, const std::string_view object_type, const _Strv object_name, const basic_address<void> object_ptr) noexcept;

struct logs_writer
{
	void operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry, const _Strv module_name) const noexcept;

	template<typename FnT>
	void operator()(const wp::found_export<FnT> ex, const _Strv module_name, const _Strv export_name) const noexcept
	{
		console_log(module_name, "export", export_name, ex.unknown);
	}

	void operator()(IMAGE_SECTION_HEADER* const sec, const _Strv module_name, const std::string_view section_name) const noexcept;

	template<typename T>
	void operator()(const wp::found_vtable<T> vt, const _Strv module_name, const _Strv vtable_name) const noexcept
	{
		console_log(module_name, "vtable", vtable_name, vt.ptr);
	}
};

uint8_t* find_signature_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig) noexcept;
void* find_interface_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const basic_address<void> create_interface_fn, const std::string_view name) noexcept;

using cheat::tools::csgo_object_name_uni;
using nstd::text::chars_cache;

template<class Gm>
class interface_finder
{
	[[no_unique_address]]
	Gm game_module_;
	basic_address<void> addr_;

public:
	interface_finder(const basic_address<void> addr, Gm&& gm = {})
		:addr_(addr), game_module_(std::move(gm))
	{
	}

	template<class T>
	operator T* () const noexcept
	{
		T* const ptr = addr_;
		console_log(game_module_._Name( ), "interface", csgo_object_name_uni<T>( ), ptr);
		return ptr;
	}

	template<size_t Idx>
	interface_finder& deref( ) noexcept
	{
		addr_ = addr_.deref<Idx>( ); return *this;
	}

	interface_finder& plus(ptrdiff_t offset) noexcept
	{
		addr_ = addr_.plus(offset); return *this;
	}

	interface_finder& minus(ptrdiff_t offset) noexcept
	{
		addr_ = addr_.minus(offset); return *this;
	}

	interface_finder& multiply(ptrdiff_t value) noexcept
	{
		addr_ = addr_.multiply(value); return *this;
	}

	interface_finder& divide(ptrdiff_t value) noexcept
	{
		addr_ = addr_.divide(value); return *this;
	}

	interface_finder& operator[](ptrdiff_t value) noexcept
	{
		addr_ = addr_[value]; return *this;
	}
};

template<class Gm>
interface_finder(Gm)->interface_finder<Gm>;

template<chars_cache Name>
struct game_module
{
	constexpr _Strv _Name( ) const noexcept
	{
		return Name.view( );
	}

	interface_finder<game_module> _Ifc_finder(const basic_address<void> addr) const noexcept
	{
		return addr;
	}

	//---

	template<typename FnT, chars_cache Export>
	FnT find_export( ) const noexcept
	{
		return wp::find_export<FnT, Name, Export, logs_writer>( );
	}

	template<chars_cache Section>
	auto find_section( ) const noexcept
	{
		return wp::find_section<Name, Section, logs_writer>( );
	}

	template<typename T>
	T* find_vtable( ) const noexcept
	{
		static const auto found = wp::find_vtable<logs_writer>(wp::find_module<Name, logs_writer>( ),
															   this->_Name( ),
															   csgo_object_name_uni<T>( ));
		return static_cast<T*>(found);
	}

	template<chars_cache Sig>
	basic_address<void> find_signature( ) const noexcept
	{
		static const auto found = find_signature_impl(wp::find_module<Name, logs_writer>( ),
													  Sig.view( ));
		return found;
	}

	template<chars_cache IfcName>
	basic_address<void> find_interface( ) const noexcept
	{
		static const auto found = find_interface_impl(wp::find_module<Name, logs_writer>( ),
													  this->find_export<void*, "CreateInterface">( ),
													  IfcName.view( ));
		return found;
	}

	template<chars_cache Sig>
	auto find_interface_sig( ) const noexcept
	{
		return _Ifc_finder(this->find_signature<Sig>( ));
	}

};

struct current_module
{
	_Strv _Name( ) const noexcept;
	interface_finder<current_module> _Ifc_finder(const basic_address<void> addr) const noexcept;
};

#define MAKE_DLL_NAME(_NAME_) #_NAME_".dll"

#ifdef _UNICODE
#define DLL_NAME(_NAME_) NSTD_CONCAT(L,MAKE_DLL_NAME(_NAME_))
#else
#define DLL_NAME(_NAME_) MAKE_DLL_NAME(_NAME_)
#endif

export namespace cheat::csgo_modules
{
	inline constexpr current_module current;

#define CHEAT_GAME_MODULE(_NAME_)\
	inline constexpr game_module<DLL_NAME(_NAME_)> _NAME_;

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
