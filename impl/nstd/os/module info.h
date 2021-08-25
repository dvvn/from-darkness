#pragma once

#include "module info/exports_storage.h"
#include "module info/sections_storage.h"
#include "module info/vtables_storage.h"

#include "nstd/one_instance.h"

namespace nstd::os
{
	class module_info final: sections_storage, exports_storage, vtables_storage
	{
		LDR_DATA_TABLE_ENTRY* ldr_entry;
		IMAGE_DOS_HEADER*     dos;
		IMAGE_NT_HEADERS*     nt;

		std::wstring name_;
		bool         name_is_unicode_;

	protected:
		module_info*       root_class( ) override;
		const module_info* root_class( ) const override;

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

		std::wstring_view work_dir( ) const;
		std::wstring_view full_path( ) const;
		std::wstring_view raw_name( ) const;

		const std::wstring& name( ) const;
		bool                name_is_unicode( ) const;

		sections_storage&       sections( );
		const sections_storage& sections_view( ) const;

		exports_storage&       exports( );
		const exports_storage& exports_view( ) const;

		vtables_storage&       vtables( );
		const vtables_storage& vtables_view( ) const;
	};

	class modules_storage
	{
	public:
		modules_storage(modules_storage&&)            = default;
		modules_storage& operator=(modules_storage&&) = default;

		modules_storage(const modules_storage&)            = delete;
		modules_storage& operator=(const modules_storage&) = delete;

		using storage_type = std::list<module_info>;

		modules_storage( ) = default;
		modules_storage& update(bool force = false);

		module_info& current( ) const;
		module_info& owner( );

		storage_type& all(bool update = false);

		template <typename Pr, class Pj = std::identity>
		module_info* find(Pr pred, Pj proj = { })
		{
			auto found = ranges::find_if(storage_, pred, proj);
			if (found == storage_.end( ))
				return nullptr;

			return std::addressof(*found);
		}

	private:
		template <typename Ret, typename T, typename Proj>
		module_info* find_unwrap(T&& val, Proj&& proj)
		{
			auto found = ranges::find(storage_, val, proj);
			if (found == storage_.end( ))
				return nullptr;

			return std::addressof(*found);
		}

	public:
		template <typename Ret, std::equality_comparable_with<Ret> T>
		module_info* find(Ret (module_info::*proj)( ), T&& val)
		{
			return this->find_unwrap<Ret>(std::forward<T>(val), proj);
		}

		template <typename Ret, std::equality_comparable_with<Ret> T>
		module_info* find(Ret (module_info::*proj)( ) const, T&& val)
		{
			return this->find_unwrap<Ret>(std::forward<T>(val), proj);
		}

	private:
		storage_type storage_;
		module_info* current_cached_ = nullptr;
	};

	namespace detail
	{
		struct all_modules_impl: modules_storage
		{
			all_modules_impl( )
			{
				this->update( );
			}
		};
	}

	using all_modules = one_instance<detail::all_modules_impl>;
}
