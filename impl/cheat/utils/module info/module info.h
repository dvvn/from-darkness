#pragma once

#include "exports_storage.h"
#include "sections_storage.h"
#include "vtables_storage.h"

#include "cheat/utils/one_instance.h"

namespace cheat::utl
{
	class module_info final: detail::sections_storage_ex<0>,
							 detail::exports_storage_ex<sizeof(detail::sections_storage)>,
							 detail::vtables_storage_ex<sizeof(detail::sections_storage) + sizeof(detail::exports_storage)>
	{
		LDR_DATA_TABLE_ENTRY* ldr_entry;
		IMAGE_DOS_HEADER*     dos;
		IMAGE_NT_HEADERS*     nt;

		string  name_;
		wstring name_wide_;

	public:
		~module_info( ) override = default;

		module_info(LDR_DATA_TABLE_ENTRY* ldr_entry, IMAGE_DOS_HEADER* dos, IMAGE_NT_HEADERS* nt);

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

		using raw_name_value_type = std::remove_pointer_t<decltype(ldr_entry->FullDllName.Buffer)>;
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

		detail::sections_storage&       sections( );
		const detail::sections_storage& sections( ) const;

		detail::exports_storage&       exports( );
		const detail::exports_storage& exports( ) const;

		detail::vtables_storage&       vtables( );
		const detail::vtables_storage& vtables( ) const;
	};

	using modules_storage_container = /*stable_*/vector<module_info>;
	class modules_storage: public noncopyable
	{
	public:
		modules_storage(modules_storage&&)            = default;
		modules_storage& operator=(modules_storage&&) = default;

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
