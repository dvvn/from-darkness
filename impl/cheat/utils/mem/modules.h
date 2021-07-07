#pragma once

#include "modules/exports_storage.h"
#include "modules/sections_storage.h"
#include "modules/vtables_storage.h"

#include "cheat/utils/one_instance.h"
//#include "cheat::utl/winapi/lib_load.h"

namespace cheat::utl::mem
{
	class module_info
	{
		/*friend class modules_storage;

		cheat::utl::winapi::module_handle manual_handle_;*/

		struct
		{
			LDR_DATA_TABLE_ENTRY* ldr_entry;
			IMAGE_DOS_HEADER*     dos;
			IMAGE_NT_HEADERS*     nt;
		}                         info__;

		struct
		{
			string  name;
			wstring name_wide;

			sections_storage sections;
			exports_storage  exports;
			vtables_storage  vtables;
		}                    members__;

		void Fix_vtables_cache_sections_( );

	public:
		module_info(LDR_DATA_TABLE_ENTRY* ldr_entry, IMAGE_DOS_HEADER* dos, IMAGE_NT_HEADERS* nt);

		module_info(const module_info&) = delete;
		module_info& operator =(const module_info&)= delete;

		module_info(module_info&& other) noexcept;
		module_info& operator=(module_info&& other) noexcept;

	protected:
		module_info( ) = default;

	public:
		//module handle
		address      base( ) const;
		memory_block mem_block( ) const;

		// ReSharper disable CppInconsistentNaming

		IMAGE_DOS_HEADER* DOS( ) const;
		IMAGE_NT_HEADERS* NT( ) const;

		// ReSharper restore CppInconsistentNaming

		DWORD check_sum( ) const;
		DWORD code_size( ) const;
		DWORD image_size( ) const;

		using raw_name_value_type = std::remove_pointer_t<decltype(info__.ldr_entry->FullDllName.Buffer)>;
		using raw_name_type = basic_string_view<raw_name_value_type>;

		raw_name_type work_dir( ) const;
		raw_name_type full_path( ) const;
		raw_name_type raw_name( ) const;

		const string& name( );
		const string& name( ) const;

		const wstring& name_wide( );
		const wstring& name_wide( ) const;

		bool name_contains_unicode( );
		bool name_contains_unicode( ) const;

		sections_storage&       sections( );
		const sections_storage& sections( ) const;

		exports_storage&       exports( );
		const exports_storage& exports( ) const;

		vtables_storage&       vtables( );
		const vtables_storage& vtables( ) const;
	};

	using modules_storage_container = /*stable_*/vector<module_info>;
	class modules_storage: public noncopyable
	{
	public:
		modules_storage(modules_storage&&) = default;
		modules_storage& operator=(modules_storage&&)= default;

		modules_storage( );

		modules_storage& update(bool force = false);

		module_info&       current( );
		const module_info& current( ) const;

		module_info&       owner( );
		const module_info& owner( ) const;

		//module_info &load(const boost::filesystem::path&from,DWORD flags);

		modules_storage_container&       all(bool update = false);
		const modules_storage_container& all( ) const;

		module_info* find(const string_view& name);
		module_info* find(const wstring_view& name);

	protected:
		void Empty_assert( ) const;

	private:
		module_info*              current_cached__;
		modules_storage_container storage__;
	};

	using all_modules = one_instance<modules_storage>;
}
