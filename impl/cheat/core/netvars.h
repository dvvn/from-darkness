#pragma once

#include "service.h"

#include "cheat/sdk/datamap.hpp"
#include "cheat/sdk/Recv.hpp"

namespace cheat
{
	namespace detail::netvars
	{
		template <typename T>
		using storage_type = utl::ordered_map_fast<utl::string, T>; //it ok, we never erase

		class netvar_source
		{
		public:
			netvar_source(csgo::RecvProp* ptr);
			netvar_source(csgo::typedescription_t* ptr);

			void*       get( ) const;
			const char* name( ) const;

		private:
			utl::variant<utl::reference_wrapper<csgo::RecvProp>, utl::reference_wrapper<csgo::typedescription_t>> obj__;
		};

		struct netvar_prop
		{
			using offset_type = int;

			offset_type offset;

			offset_type   level;
			netvar_source source;
		};

		class dumped_class
		{
		public:
			const netvar_prop*            find(const utl::string_view& name) const;
			utl::pair<netvar_prop&, bool> try_emplace(const utl::string_view& name, netvar_prop&& prop);
			const netvar_prop&            at(const utl::string_view& name) const;

		private:
			storage_type<netvar_prop> props__;
		};

		using classes_storage = storage_type<dumped_class>;
	}

	class netvars final: public service_shared<netvars, service_mode::async>
	{
	public:
		~netvars( ) override;
		netvars( );

		const detail::netvars::netvar_prop& at(const utl::string_view& class_name, const utl::string_view& prop_name) const;

	protected:
		void        Load( ) override;
		utl::string Get_loaded_message( ) const override;
		void        Post_load( ) override;

	private:
		detail::netvars::classes_storage data__;
	};
}
