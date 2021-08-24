#pragma once

#include "settings detail.h"
#include "cheat/core/service.h"

namespace cheat
{
	class settings_data: protected gui::tools::string_wrapper_base
	{
	public:
		friend class settings;

		using tree_type = nlohmann::json;

		virtual ~settings_data( ) = default;

		settings_data(const std::string_view& name);
		settings_data(std::wstring&& name);

		virtual bool save(const std::wstring_view& name) const;
		bool         save(const std::wstring_view& name);
		virtual bool load(const std::wstring_view& name);
		bool         remove(const std::wstring_view& name) const;
		bool         remove_all( ) const;

		virtual void update( ) =0;

		using string_wrapper_base::name;

		std::wstring_view path( ) const;

	protected:
		std::wstring Generate_path(const std::wstring_view& name) const;

		tree_type             tree_;
		std::filesystem::path path_;
	};

	class settings final: public gui::objects::empty_page, public settings_data,
						  public service<settings>
	{
	public:
		settings( );

		void update( ) override;
		void render( ) override;

	protected:
		bool load_impl( ) override;

	private:
#if 0
		bool merge__ = false;
#endif

		settings_detail::folder_with_configs_mgr mgr__;
	};
}
