#pragma once

#include "nstd/address.h"

namespace nstd::os
{
	class module_info;

	namespace detail
	{
		struct module_data_mgr_root_getter
		{
		protected:
			~module_data_mgr_root_getter( ) = default;
			virtual module_info*       root_class( ) =0;
			virtual const module_info* root_class( ) const =0;
		};

		class module_data_mgr_base: public virtual module_data_mgr_root_getter
		{
		protected:
			virtual ~module_data_mgr_base( ) = default;

		public:
			using path_type = std::filesystem::path;
			using ptree_type = nlohmann::json;

			virtual bool load(const path_type& file) =0;
			virtual bool save_to_file(const path_type& file) const =0;

			//virtual path_type get_file_name( ) const =0;

		protected:
			address           base_addr( ) const;
			IMAGE_NT_HEADERS* nt_header( ) const;

			bool write_from_storage(const path_type& file, const ptree_type& storage) const;
			bool read_to_storage(const path_type& file, ptree_type& storage) const;
		};

		template <typename T>
		class module_data_mgr: public module_data_mgr_base
		{
		public:
			using cache_type = unordered_map<std::string, T>;

			cache_type& get_cache( )
			{
				return cache_;
			}

			bool load(const path_type& file = { }) final
			{
				if (cache_.empty( ))
				{
					auto storage    = ptree_type( );
					auto temp_cache = cache_type( );

					if (module_data_mgr_base::read_to_storage(file, storage))
					{
						temp_cache.reserve(storage.size( ));
						if (this->load_from_file(temp_cache, std::move(storage)))
						{
							cache_ = std::move(temp_cache);
							return true;
						}
					}

					if (!this->load_from_memory(temp_cache))
						return false;

					cache_ = std::move(temp_cache);
				}

				if (file.empty( ))
					return true;

				return this->save_to_file(file);
			}

			bool save_to_file(const path_type& file) const final
			{
				if (file.empty( ))
					return false;
				if (cache_.empty( ))
					return false;

				auto storage = ptree_type( );
				if (!this->read_to_storage(cache_, storage))
					return false;

				return write_from_storage(file, storage);
			}

			virtual bool load_from_memory(cache_type& cache) =0;
			virtual bool load_from_file(cache_type& cache, ptree_type&& storage) =0;
		protected:
			virtual bool read_to_storage(const cache_type& cache, ptree_type& storage) const =0;

		private:
			cache_type cache_;
		};
	}
}
