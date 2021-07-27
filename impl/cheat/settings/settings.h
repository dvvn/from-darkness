#pragma once

#include "settings detail.h"
#include "cheat/core/service.h"

namespace cheat
{
	class settings_data: protected gui::tools::string_wrapper_base
	{
	public:
		friend class settings;

		using tree_type = utl::property_tree::wiptree;

		virtual ~settings_data( ) = default;

		settings_data(const utl::string_view& name);
		settings_data(utl::wstring&& name);

		virtual bool save(const utl::wstring_view& name) const;
		bool save(const utl::wstring_view& name);
		virtual bool load(const utl::wstring_view& name);
		bool remove(const utl::wstring_view& name) const;
		bool remove_all( ) const;

		virtual void update( ) =0;

		using string_wrapper_base::name;

		utl::wstring_view path( ) const;

	protected:
		template <typename T>
		void Load_or_create(const utl::string_view& name, T& value)
		{
			Load_or_create<T>(utl::wstring(name.begin( ), name.end( )), value);
		}

		template <typename T>
		void Load_or_create(const utl::property_tree::wpath& name, T& value)
		{
			if (const auto child = tree_.get_child_optional(name); !child)
				tree_.put(name, value);
			else
				value = child->get_value<T>( );
		}

		utl::wstring Generate_path(const utl::wstring_view& name) const;

		tree_type tree_;
		utl::filesystem::path path_;
	};

	class settings final: public gui::objects::empty_page, public settings_data,
						  public service<settings>
	{
	public:
		settings( );

		void update( ) override;
		void render( ) override;

	protected:
		bool Do_load( ) override;

	private:
#if 0
		bool merge__ = false;
#endif

		detail::settings::folder_with_configs_mgr mgr__;
	};
}
