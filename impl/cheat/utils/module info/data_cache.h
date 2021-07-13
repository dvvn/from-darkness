#pragma once

#include "cheat/utils/address.h"

namespace cheat::utl
{
	enum module_info_rw_result:uint8_t
	{
		unknown=0,
		success,
		error,
		nothing
	};

	namespace detail
	{
		using path_type = filesystem::path;
		using ptree_type = property_tree::basic_ptree<string, string>;

		class data_cache_base
		{
		public:
			friend class data_cache_from_file;

			virtual ~data_cache_base( );

			data_cache_base(address addr, IMAGE_NT_HEADERS* nt);

			module_info_rw_result load( );
			module_info_rw_result load_from_memory( );
			bool                  change_base_address(address new_addr);

		protected:
			virtual bool Cache_empty_impl( ) const = 0;
			virtual void Cache_reserve_impl(size_t capacity) = 0;

			virtual module_info_rw_result Load_from_memory_impl( ) = 0;
			virtual void                  Change_base_address_impl(address new_addr) = 0;

			void Empty_cache_assert( ) const;

			address           base_address = nullptr;
			IMAGE_NT_HEADERS* nt = nullptr;
		};

		template <typename T>
		class data_cache_from_memory: public data_cache_base
		{
		public:
			using cache_type = unordered_map<string, T>;

			data_cache_from_memory(address addr, IMAGE_NT_HEADERS* nt) : data_cache_base(move(addr), nt)
			{
			}

			const cache_type& get_cache( ) const
			{
				Empty_cache_assert( );
				return data_cache;
			}

		protected:
			bool Cache_empty_impl( ) const final
			{
				return data_cache.empty( );
			}

			void Cache_reserve_impl(size_t capacity) final
			{
				return data_cache.reserve(capacity);
			}

			cache_type data_cache;
		};

		class data_cache_from_file
		{
		protected:
			virtual ~data_cache_from_file( ) = default;

			module_info_rw_result Load_from_file(data_cache_base& vtable1, const path_type& full_path);
			module_info_rw_result Write_to_file(const data_cache_base& vtable1, const filesystem::path& full_path) const;

			virtual module_info_rw_result Write_to_file_impl(ptree_type& cache) const = 0;
			virtual module_info_rw_result Load_from_file_impl(const ptree_type& cache) = 0;

			module_info_rw_result Load(data_cache_base& vtable1, const path_type& full_path);
		};

		template <typename T>
		class data_cache_from_anywhere: public data_cache_from_memory<T>, protected data_cache_from_file
		{
		public:
			using from_memory = data_cache_from_memory<T>;

			data_cache_from_anywhere(address addr, IMAGE_NT_HEADERS* nt) : from_memory(move(addr), nt)
			{
			}

			module_info_rw_result load_from_file(const path_type& full_path)
			{
				return data_cache_from_file::Load_from_file(*this, full_path);
			}

			module_info_rw_result write_to_file(const path_type& full_path) const
			{
				return data_cache_from_file::Write_to_file(*this, full_path);
			}

			module_info_rw_result load(const path_type& full_path)
			{
				return data_cache_from_file::Load(*this, full_path);
			}

			using from_memory::load;
		};
	}
}
